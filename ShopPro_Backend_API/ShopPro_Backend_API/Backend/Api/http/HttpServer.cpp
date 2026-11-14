#include "HttpServer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include "../Support.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketHandle = SOCKET;
static constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
static void closeSocket(SocketHandle s) { closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
using SocketHandle = int;
static constexpr SocketHandle kInvalidSocket = -1;
static void closeSocket(SocketHandle s) { close(s); }
#endif

namespace shoppro {
namespace {

constexpr std::size_t kMaxHeaderBytes = 16 * 1024;
constexpr std::size_t kMaxBodyBytes = 1024 * 1024;
constexpr std::size_t kMaxFileBytes = 20 * 1024 * 1024;
constexpr int kMaxRequestsPerConnection = 100;
constexpr int kSocketTimeoutSeconds = 15;

std::mutex gLogMutex;

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

const char* reasonPhrase(int status) {
    switch (status) {
        case 100: return "Continue";
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 411: return "Length Required";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 429: return "Too Many Requests";
        case 431: return "Request Header Fields Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 503: return "Service Unavailable";
        default: return "Error";
    }
}

struct Wire {                 // phan hoi o dang byte
    int status = 200;
    std::string contentType;
    std::string body;
    std::map<std::string, std::string> headers;
    bool headOnly = false;
};

std::string serialize(const Wire& wire, bool keepAlive) {
    std::ostringstream out;
    out << "HTTP/1.1 " << wire.status << ' ' << reasonPhrase(wire.status) << "\r\n"
        << "Content-Type: " << wire.contentType << "\r\n"
        << "Content-Length: " << wire.body.size() << "\r\n"
        << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "X-Frame-Options: DENY\r\n"
        << "Referrer-Policy: same-origin\r\n"
        << "Content-Security-Policy: default-src 'self'; img-src 'self' https: data:; style-src 'self' 'unsafe-inline'; "
           "script-src 'self'; connect-src 'self'; frame-ancestors 'none'; base-uri 'self'; form-action 'self'\r\n";
    for (const auto& header : wire.headers) out << header.first << ": " << header.second << "\r\n";
    out << "\r\n";
    if (!wire.headOnly) out << wire.body;
    return out.str();
}

bool sendAll(SocketHandle socket, const std::string& data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        int flags = 0;
#ifdef MSG_NOSIGNAL
        flags = MSG_NOSIGNAL;
#endif
#ifdef _WIN32
        const int count = send(socket, data.data() + sent, static_cast<int>(std::min<std::size_t>(data.size() - sent, 1 << 20)), flags);
#else
        const auto count = send(socket, data.data() + sent, data.size() - sent, flags);
#endif
        if (count <= 0) return false;
        sent += static_cast<std::size_t>(count);
    }
    return true;
}

Wire errorPage(int status, const std::string& message) {
    Wire wire;
    wire.status = status;
    wire.contentType = "application/json; charset=utf-8";
    wire.body = Json{{"error", message}}.dump(-1, ' ', false, Json::error_handler_t::replace);
    wire.headers["Cache-Control"] = "no-store";
    return wire;
}

std::string mimeType(const std::string& extension) {
    static const std::map<std::string, std::string> types{
        {".html", "text/html; charset=utf-8"}, {".css", "text/css; charset=utf-8"},
        {".js", "text/javascript; charset=utf-8"}, {".mjs", "text/javascript; charset=utf-8"},
        {".json", "application/json; charset=utf-8"}, {".svg", "image/svg+xml"},
        {".png", "image/png"}, {".jpg", "image/jpeg"}, {".jpeg", "image/jpeg"}, {".webp", "image/webp"},
        {".gif", "image/gif"}, {".ico", "image/x-icon"}, {".txt", "text/plain; charset=utf-8"},
        {".woff2", "font/woff2"}, {".woff", "font/woff"}, {".map", "application/json"}};
    const auto found = types.find(extension);
    return found == types.end() ? "application/octet-stream" : found->second;
}

// ------------------------------ tep tinh ------------------------------
Wire serveStatic(const ServerConfig& config, const HttpRequest& request) {
    if (config.webRoot.empty()) return errorPage(404, "Không tìm thấy trang.");
    if (request.method != "GET" && request.method != "HEAD") {
        Wire wire = errorPage(405, "Chỉ hỗ trợ GET/HEAD cho trang tĩnh.");
        wire.headers["Allow"] = "GET, HEAD";
        return wire;
    }
    std::string path = request.target.substr(0, request.target.find('?'));
    try {
        path = text::urlDecode(path);
    } catch (const ApiError&) {
        return errorPage(400, "Đường dẫn không hợp lệ.");
    }
    if (path.find('\0') != std::string::npos || path.find('\\') != std::string::npos) return errorPage(404, "Không tìm thấy trang.");
    for (const auto& segment : text::split(path, '/')) {
        if (segment == ".." || (!segment.empty() && segment[0] == '.')) return errorPage(404, "Không tìm thấy trang.");
    }

    try {
        const auto root = std::filesystem::weakly_canonical(config.webRoot);
        auto candidate = std::filesystem::weakly_canonical(root / path.substr(path.empty() ? 0 : 1));
        const auto rootText = root.generic_string();
        const auto candidateText = candidate.generic_string();
        if (candidateText.compare(0, rootText.size(), rootText) != 0 ||
            (candidateText.size() > rootText.size() && candidateText[rootText.size()] != '/')) {
            return errorPage(404, "Không tìm thấy trang.");
        }
        if (std::filesystem::is_directory(candidate)) {
            if (path.back() != '/') {   // /admin -> /admin/ de duong dan tuong doi hoat dong
                Wire redirect;
                redirect.status = 301;
                redirect.contentType = "text/plain; charset=utf-8";
                redirect.headers["Location"] = path + "/";
                return redirect;
            }
            candidate /= "index.html";
        }
        if (!std::filesystem::is_regular_file(candidate)) return errorPage(404, "Không tìm thấy trang.");
        const auto size = std::filesystem::file_size(candidate);
        if (size > kMaxFileBytes) return errorPage(404, "Không tìm thấy trang.");
        std::ifstream input(candidate, std::ios::binary);
        if (!input) return errorPage(404, "Không tìm thấy trang.");
        Wire wire;
        wire.status = 200;
        wire.contentType = mimeType(lowerAscii(candidate.extension().string()));
        wire.body.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
        wire.headOnly = request.method == "HEAD";
        const bool immutableAsset = wire.contentType.rfind("image/", 0) == 0 || wire.contentType.rfind("font/", 0) == 0;
        wire.headers["Cache-Control"] = immutableAsset ? "public, max-age=86400" : "no-cache";
        return wire;
    } catch (const std::filesystem::filesystem_error&) {
        return errorPage(404, "Không tìm thấy trang.");
    }
}

// ------------------------------ doc yeu cau ------------------------------
enum class ReadResult { Ok, Closed, Error };

struct ConnectionState {
    SocketHandle socket;
    std::string buffer;         // du lieu da nhan nhung chua xu ly (pipelining)
    std::string peerIp;
};

// Nhan them du lieu vao buffer. false = dong/het gio/loi (timedOut cho biet ly do)
bool receiveMore(ConnectionState& conn, bool& timedOut) {
    std::array<char, 8192> chunk{};
#ifdef _WIN32
    const int count = recv(conn.socket, chunk.data(), static_cast<int>(chunk.size()), 0);
    timedOut = count < 0 && WSAGetLastError() == WSAETIMEDOUT;
#else
    const auto count = recv(conn.socket, chunk.data(), chunk.size(), 0);
    timedOut = count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK);
#endif
    if (count <= 0) return false;
    conn.buffer.append(chunk.data(), static_cast<std::size_t>(count));
    return true;
}

std::string sanitizeIp(const std::string& value) {
    std::string result;
    for (const char ch : value) {
        if (std::isxdigit(static_cast<unsigned char>(ch)) || ch == '.' || ch == ':') result.push_back(ch);
        if (result.size() >= 45) break;
    }
    return result;
}

ReadResult readRequest(ConnectionState& conn, const ServerConfig& config, HttpRequest& request, bool& keepAlive,
                       Wire& failure) {
    std::size_t headerEnd = std::string::npos;
    while ((headerEnd = conn.buffer.find("\r\n\r\n")) == std::string::npos) {
        if (conn.buffer.size() > kMaxHeaderBytes) {
            failure = errorPage(431, "Tiêu đề yêu cầu quá lớn.");
            return ReadResult::Error;
        }
        bool timedOut = false;
        if (!receiveMore(conn, timedOut)) {
            if (conn.buffer.empty()) return ReadResult::Closed;          // keep-alive het han / client dong
            if (timedOut) failure = errorPage(408, "Hết thời gian chờ yêu cầu.");
            else return ReadResult::Closed;
            return ReadResult::Error;
        }
    }
    if (headerEnd > kMaxHeaderBytes) {
        failure = errorPage(431, "Tiêu đề yêu cầu quá lớn.");
        return ReadResult::Error;
    }
    const std::string head = conn.buffer.substr(0, headerEnd);
    conn.buffer.erase(0, headerEnd + 4);

    std::istringstream lines(head);
    std::string requestLine;
    std::getline(lines, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') requestLine.pop_back();
    std::istringstream parts(requestLine);
    std::string method, target, version, extra;
    parts >> method >> target >> version;
    if (method.empty() || target.empty() || version.empty() || (parts >> extra)) {
        failure = errorPage(400, "Dòng yêu cầu không hợp lệ.");
        return ReadResult::Error;
    }
    if (method.size() > 10 || !std::all_of(method.begin(), method.end(), [](unsigned char c) { return std::isupper(c) != 0; })) {
        failure = errorPage(400, "Phương thức không hợp lệ.");
        return ReadResult::Error;
    }
    if (version != "HTTP/1.1" && version != "HTTP/1.0") {
        failure = errorPage(400, "Phiên bản HTTP không được hỗ trợ.");
        return ReadResult::Error;
    }
    if (target.size() > 4096) {
        failure = errorPage(414, "Đường dẫn quá dài.");
        return ReadResult::Error;
    }
    if (target[0] != '/') {
        failure = errorPage(400, "Chỉ hỗ trợ đường dẫn dạng /path.");
        return ReadResult::Error;
    }
    request.method = method;
    request.target = target;
    request.headers.clear();
    request.body.clear();

    std::string line;
    int headerCount = 0;
    while (std::getline(lines, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        const auto colon = line.find(':');
        if (colon == std::string::npos || colon == 0 || ++headerCount > 100) {
            failure = errorPage(400, "Tiêu đề yêu cầu không hợp lệ.");
            return ReadResult::Error;
        }
        const std::string name = lowerAscii(text::trim(line.substr(0, colon)));
        const std::string value = text::trim(line.substr(colon + 1));
        if (name == "content-length" && request.headers.count(name) && request.headers[name] != value) {
            failure = errorPage(400, "Content-Length mâu thuẫn.");
            return ReadResult::Error;
        }
        request.headers[name] = value;
    }

    if (request.headers.count("transfer-encoding")) {
        failure = errorPage(501, "Máy chủ chưa hỗ trợ Transfer-Encoding. Hãy dùng Content-Length.");
        return ReadResult::Error;
    }

    std::size_t contentLength = 0;
    if (request.headers.count("content-length")) {
        const std::string& value = request.headers["content-length"];
        if (value.empty() || value.size() > 10 || value.find_first_not_of("0123456789") != std::string::npos) {
            failure = errorPage(400, "Content-Length không hợp lệ.");
            return ReadResult::Error;
        }
        contentLength = static_cast<std::size_t>(std::stoull(value));
        if (contentLength > kMaxBodyBytes) {
            failure = errorPage(413, "Nội dung yêu cầu quá lớn (tối đa 1 MB).");
            return ReadResult::Error;
        }
    }
    if (contentLength > 0 && lowerAscii(request.headers["expect"]) == "100-continue") {
        sendAll(conn.socket, "HTTP/1.1 100 Continue\r\n\r\n");
    }
    while (conn.buffer.size() < contentLength) {
        bool timedOut = false;
        if (!receiveMore(conn, timedOut)) {
            failure = errorPage(timedOut ? 408 : 400, timedOut ? "Hết thời gian chờ nội dung." : "Kết nối bị ngắt giữa chừng.");
            return ReadResult::Error;
        }
    }
    request.body = conn.buffer.substr(0, contentLength);
    conn.buffer.erase(0, contentLength);

    // IP client
    request.ip = conn.peerIp;
    if (config.trustProxy && request.headers.count("x-forwarded-for")) {
        const std::string chain = request.headers["x-forwarded-for"];
        const auto comma = chain.rfind(',');   // phan tu cuoi cung do proxy tin cay them vao
        const std::string candidate = sanitizeIp(text::trim(comma == std::string::npos ? chain : chain.substr(comma + 1)));
        if (!candidate.empty()) request.ip = candidate;
    }

    const std::string connection = lowerAscii(request.headers.count("connection") ? request.headers["connection"] : "");
    keepAlive = version == "HTTP/1.1" ? connection != "close" : connection == "keep-alive";
    return ReadResult::Ok;
}

// ------------------------------ vong doi ket noi ------------------------------
void setTimeouts(SocketHandle socket) {
#ifdef _WIN32
    const DWORD timeout = kSocketTimeoutSeconds * 1000;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof timeout);
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof timeout);
#else
    timeval timeout{};
    timeout.tv_sec = kSocketTimeoutSeconds;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);
#endif
}

void serveConnection(SocketHandle socket, std::string peerIp, const ServerConfig& config, const ApiHandler& api) {
    setTimeouts(socket);
    ConnectionState conn{socket, {}, std::move(peerIp)};
    for (int served = 0; served < kMaxRequestsPerConnection; ++served) {
        HttpRequest request;
        bool keepAlive = false;
        Wire failure;
        const ReadResult result = readRequest(conn, config, request, keepAlive, failure);
        if (result == ReadResult::Closed) break;
        if (result == ReadResult::Error) {
            sendAll(socket, serialize(failure, false));
            break;
        }

        const auto started = std::chrono::steady_clock::now();
        Wire wire;
        const bool isApi = request.target == "/api" || request.target.rfind("/api/", 0) == 0 || request.target.rfind("/api?", 0) == 0;
        if (isApi) {
            HttpResponse response;
            try {
                response = api(request);
            } catch (const std::exception&) {
                response = {500, {{"error", "Không thể hoàn tất yêu cầu."}}, {}};
            }
            wire.status = response.status;
            wire.contentType = "application/json; charset=utf-8";
            wire.body = response.status == 204 ? std::string() : response.body.dump(-1, ' ', false, Json::error_handler_t::replace);
            wire.headers = response.headers;
            wire.headers["Cache-Control"] = "no-store";
            wire.headOnly = request.method == "HEAD";
        } else {
            wire = serveStatic(config, request);
        }
        const bool sent = sendAll(socket, serialize(wire, keepAlive && served + 1 < kMaxRequestsPerConnection));
        if (config.logRequests && (isApi || wire.status >= 400)) {
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();
            std::lock_guard<std::mutex> lock(gLogMutex);
            std::cout << "[" << timeutil::nowIso().substr(11, 8) << "] " << request.method << ' '
                      << request.target.substr(0, 120) << " -> " << wire.status << " (" << ms << " ms) " << request.ip << std::endl;
        }
        if (!sent || !keepAlive) break;
    }
    closeSocket(socket);
}

}  // namespace

int runHttpServer(const ServerConfig& config, const ApiHandler& api, const std::atomic<bool>& stopFlag) {
#ifdef _WIN32
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Không khởi tạo được Winsock.\n";
        return 2;
    }
#endif
    const SocketHandle listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == kInvalidSocket) {
        std::cerr << "Không tạo được socket.\n";
        return 2;
    }
#ifndef _WIN32
    const int reuse = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
#endif
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(config.port);
    const std::string host = config.host == "localhost" ? "127.0.0.1" : config.host;
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        std::cerr << "Địa chỉ host không hợp lệ: " << config.host << '\n';
        closeSocket(listener);
        return 2;
    }
    if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof address) != 0 || listen(listener, 128) != 0) {
        std::cerr << "Không mở được cổng " << config.port << " (đã có chương trình khác dùng cổng này?).\n";
        closeSocket(listener);
        return 2;
    }

    std::atomic<std::size_t> active{0};
    while (!stopFlag.load()) {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(listener, &readable);
        timeval wait{};
        wait.tv_usec = 400000;
        const int ready = select(static_cast<int>(listener) + 1, &readable, nullptr, nullptr, &wait);
        if (ready <= 0) continue;

        sockaddr_in peer{};
#ifdef _WIN32
        int peerLength = sizeof peer;
#else
        socklen_t peerLength = sizeof peer;
#endif
        const SocketHandle client = accept(listener, reinterpret_cast<sockaddr*>(&peer), &peerLength);
        if (client == kInvalidSocket) continue;
        char ip[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof ip);

        if (active.load() >= config.maxConnections) {
            sendAll(client, serialize(errorPage(503, "Máy chủ đang quá tải. Vui lòng thử lại sau."), false));
            closeSocket(client);
            continue;
        }
        ++active;
        try {
            std::thread([client, peerIp = std::string(ip), &config, &api, &active] {
                serveConnection(client, peerIp, config, api);
                --active;
            }).detach();
        } catch (const std::system_error&) {
            --active;
            closeSocket(client);
        }
    }
    closeSocket(listener);
    for (int waited = 0; active.load() > 0 && waited < 50; ++waited) {   // cho cac ket noi dang xu ly toi da 5 giay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

}  // namespace shoppro
