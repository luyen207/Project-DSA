// Kiem thu may chu HTTP bang socket that (loopback): do ben, gioi han, bao mat tep tinh, dong thoi.
// Chi chay tren Linux/macOS (dung socket POSIX); tren Windows chuong trinh bao "bo qua".
#include <iostream>

#ifdef _WIN32
int main() {
    std::cout << "test_http: bo qua (chi ho tro Linux/macOS)\n";
    return 0;
}
#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "ApiContext.h"
#include "http/HttpServer.h"

using namespace shoppro;
namespace fs = std::filesystem;

namespace {

int gChecks = 0;
int gFailures = 0;

#define CHECK(cond)                                                                       \
    do {                                                                                  \
        ++gChecks;                                                                        \
        if (!(cond)) {                                                                    \
            ++gFailures;                                                                  \
            std::cerr << "THAT BAI " << __FILE__ << ":" << __LINE__ << "  " #cond << "\n"; \
        }                                                                                 \
    } while (0)

struct Reply {
    int status = 0;
    std::map<std::string, std::string> headers;
    std::string body;
    bool connected = false;
};

int connectTo(unsigned short port) {
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    if (connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof address) != 0) {
        close(fd);
        return -1;
    }
    timeval timeout{};
    timeout.tv_sec = 5;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    return fd;
}

bool readUntil(int fd, std::string& buffer, const std::string& marker) {
    char chunk[4096];
    while (buffer.find(marker) == std::string::npos) {
        const ssize_t count = recv(fd, chunk, sizeof chunk, 0);
        if (count <= 0) return false;
        buffer.append(chunk, static_cast<std::size_t>(count));
    }
    return true;
}

// Doc MOT phan hoi tu fd (dung Content-Length). `buffer` giu phan du cho phan hoi tiep theo (pipelining).
Reply readReply(int fd, std::string& buffer, bool headOnly = false) {
    Reply reply;
    if (!readUntil(fd, buffer, "\r\n\r\n")) return reply;
    reply.connected = true;
    const std::size_t end = buffer.find("\r\n\r\n");
    const std::string head = buffer.substr(0, end);
    buffer.erase(0, end + 4);
    reply.status = std::atoi(head.substr(9, 3).c_str());
    std::size_t pos = head.find("\r\n");
    while (pos != std::string::npos && pos + 2 < head.size()) {
        const std::size_t next = head.find("\r\n", pos + 2);
        const std::string line = head.substr(pos + 2, next == std::string::npos ? std::string::npos : next - pos - 2);
        const auto colon = line.find(':');
        if (colon != std::string::npos) reply.headers[text::lower(line.substr(0, colon))] = text::trim(line.substr(colon + 1));
        pos = next;
    }
    std::size_t length = reply.headers.count("content-length") ? std::stoull(reply.headers["content-length"]) : 0;
    if (headOnly) length = 0;
    char chunk[4096];
    while (buffer.size() < length) {
        const ssize_t count = recv(fd, chunk, sizeof chunk, 0);
        if (count <= 0) break;
        buffer.append(chunk, static_cast<std::size_t>(count));
    }
    reply.body = buffer.substr(0, length);
    buffer.erase(0, length);
    return reply;
}

Reply roundTrip(unsigned short port, const std::string& raw, bool headOnly = false) {
    const int fd = connectTo(port);
    if (fd < 0) return {};
    send(fd, raw.data(), raw.size(), MSG_NOSIGNAL);
    std::string buffer;
    Reply reply = readReply(fd, buffer, headOnly);
    close(fd);
    return reply;
}

std::string get(const std::string& path, const std::string& extra = "") {
    return "GET " + path + " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n" + extra + "\r\n";
}

}  // namespace

int main() {
    // ---- thu muc web tam + tep "bi mat" nam NGOAI webRoot ----
    const fs::path temp = fs::temp_directory_path() / ("shoppro_http_" + crypto::randomHex(6));
    const fs::path web = temp / "web";
    fs::create_directories(web / "shop");
    fs::create_directories(web / ".git");
    { std::ofstream(web / "index.html") << "<h1>Trang chu</h1>"; }
    { std::ofstream(web / "shop" / "index.html") << "<h1>Cua hang</h1>"; }
    { std::ofstream(web / ".git" / "config") << "bi-mat"; }
    { std::ofstream(web / ".env") << "SECRET=1"; }
    { std::ofstream(temp / "secret.txt") << "NGOAI-WEBROOT"; }

    JsonStore store(temp / "data.json", fs::path(SHOPPRO_TEST_SEED_FILE));
    const std::int64_t shift = store.seedShiftSeconds();
    Json data = store.load();
    ActivityLog log(temp / "activity.jsonl", fs::path(SHOPPRO_TEST_SEED_FILE).parent_path() / "seed-activity.jsonl", shift);
    Ctx ctx(store, log, std::move(data));
    const Router router = buildRouter();
    const ApiHandler api = [&](const HttpRequest& request) { return router.dispatch(ctx, request); };

    ServerConfig config;
    config.host = "127.0.0.1";
    config.webRoot = web;
    config.logRequests = false;
    std::atomic<bool> stop{false};
    std::atomic<int> serverCode{-1};
    unsigned short port = 0;
    std::thread server;
    for (unsigned short candidate = 31000; candidate < 31200 && port == 0; ++candidate) {
        config.port = candidate;
        serverCode = -1;
        server = std::thread([&] { serverCode = runHttpServer(config, api, stop); });
        for (int i = 0; i < 30 && serverCode.load() == -1; ++i) {
            const int fd = connectTo(candidate);
            if (fd >= 0) { close(fd); port = candidate; break; }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (port == 0) { stop = true; server.join(); stop = false; }
    }
    if (port == 0) { std::cerr << "Khong mo duoc may chu thu nghiem\n"; return 2; }

    // 1) API + tieu de bao mat
    Reply r = roundTrip(port, get("/api/health"));
    CHECK(r.status == 200);
    CHECK(r.headers["content-type"].find("application/json") == 0);
    CHECK(r.headers["x-content-type-options"] == "nosniff");
    CHECK(r.headers["x-frame-options"] == "DENY");
    CHECK(r.headers["content-security-policy"].find("script-src 'self'") != std::string::npos);
    CHECK(r.headers["cache-control"] == "no-store");
    CHECK(Json::parse(r.body)["status"] == "ok");

    // 2) keep-alive + pipelining: hai yeu cau trong mot lan gui, hai phan hoi theo dung thu tu
    {
        const int fd = connectTo(port);
        const std::string two = "GET /api/health HTTP/1.1\r\nHost: x\r\n\r\nGET /api/categories HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n";
        send(fd, two.data(), two.size(), MSG_NOSIGNAL);
        std::string buffer;
        Reply a = readReply(fd, buffer);
        Reply b = readReply(fd, buffer);
        CHECK(a.status == 200 && a.headers["connection"] == "keep-alive");
        CHECK(b.status == 200 && Json::parse(b.body).is_array());
        close(fd);
    }

    // 3) hoi quy D2: ket noi treo (gui nua tieu de roi im lang) KHONG duoc chan khach khac
    {
        const int stalled = connectTo(port);
        const std::string half = "GET /api/health HTTP/1.1\r\nHost:";
        send(stalled, half.data(), half.size(), MSG_NOSIGNAL);
        const auto started = std::chrono::steady_clock::now();
        Reply fast = roundTrip(port, get("/api/health"));
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();
        CHECK(fast.status == 200);
        CHECK(elapsed < 1500);
        close(stalled);
    }

    // 4) gioi han kich thuoc / giao thuc
    r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nContent-Length: 5000000\r\nConnection: close\r\n\r\n");
    CHECK(r.status == 413);
    r = roundTrip(port, "GET /api/health HTTP/1.1\r\nHost: x\r\nX-Big: " + std::string(20000, 'a') + "\r\n\r\n");
    CHECK(r.status == 431);
    r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n");
    CHECK(r.status == 501);
    r = roundTrip(port, "KHONG-HOP-LE\r\n\r\n");
    CHECK(r.status == 400);
    r = roundTrip(port, "get /api/health HTTP/1.1\r\n\r\n");
    CHECK(r.status == 400);                                   // phuong thuc phai viet hoa
    r = roundTrip(port, "GET /api/health HTTP/2.0\r\n\r\n");
    CHECK(r.status == 400);
    r = roundTrip(port, "GET http://evil.example/ HTTP/1.1\r\n\r\n");
    CHECK(r.status == 400);
    r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nContent-Length: 4\r\nContent-Length: 9\r\n\r\nabcd");
    CHECK(r.status == 400);                                   // Content-Length mau thuan
    r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nContent-Length: -1\r\n\r\n");
    CHECK(r.status == 400);
    r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nContent-Length: 11\r\nConnection: close\r\n\r\n{not-json}x");
    CHECK(r.status == 400);                                   // JSON hong -> 400 (khong phai 500)

    // 5) dang nhap qua mang that (doc than yeu cau) + IP khong bi gia mao qua X-Forwarded-For
    {
        const std::string body = R"({"email":"khach@shoppro.vn","password":"sai-roi"})";
        r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nX-Forwarded-For: 6.6.6.6\r\nContent-Type: application/json\r\nContent-Length: " +
                               std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
        CHECK(r.status == 401);
        LogQuery q;
        q.action = "auth.login.failed";
        q.limit = 1;
        const Json entries = log.query(q);
        CHECK(entries["items"][0]["ip"] == "127.0.0.1");     // khong tin XFF khi khong chay sau proxy
        const std::string good = R"({"email":"khach@shoppro.vn","password":"Demo@12345"})";
        r = roundTrip(port, "POST /api/auth/login HTTP/1.1\r\nHost: x\r\nContent-Length: " + std::to_string(good.size()) +
                               "\r\nConnection: close\r\n\r\n" + good);
        CHECK(r.status == 200);
        CHECK(Json::parse(r.body)["token"].get<std::string>().size() == 64);
    }

    // 6) tep tinh + chong duyet thu muc (gui duong dan tho qua socket, khong qua trinh duyet)
    r = roundTrip(port, get("/"));
    CHECK(r.status == 200 && r.body == "<h1>Trang chu</h1>");
    CHECK(r.headers["content-type"].find("text/html") == 0);
    r = roundTrip(port, get("/shop/"));
    CHECK(r.status == 200 && r.body == "<h1>Cua hang</h1>");
    r = roundTrip(port, get("/shop"));
    CHECK(r.status == 301 && r.headers["location"] == "/shop/");
    r = roundTrip(port, get("/../secret.txt"));
    CHECK(r.status == 404 && r.body.find("NGOAI-WEBROOT") == std::string::npos);
    r = roundTrip(port, get("/%2e%2e/secret.txt"));
    CHECK(r.status == 404 && r.body.find("NGOAI-WEBROOT") == std::string::npos);
    r = roundTrip(port, get("/shop/..%2f..%2fsecret.txt"));
    CHECK(r.status == 404 && r.body.find("NGOAI-WEBROOT") == std::string::npos);
    r = roundTrip(port, get("/..\\secret.txt"));
    CHECK(r.status == 404);
    r = roundTrip(port, get("/%00"));
    CHECK(r.status == 404);
    r = roundTrip(port, get("/.env"));
    CHECK(r.status == 404 && r.body.find("SECRET") == std::string::npos);
    r = roundTrip(port, get("/.git/config"));
    CHECK(r.status == 404 && r.body.find("bi-mat") == std::string::npos);
    r = roundTrip(port, get("/khong-co.html"));
    CHECK(r.status == 404);
    r = roundTrip(port, "POST /index.html HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n");
    CHECK(r.status == 405);
    r = roundTrip(port, "HEAD / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n", true);
    CHECK(r.status == 200 && r.body.empty() && r.headers["content-length"] == "18");

    // 7) khach dong ket noi giua chung (SIGPIPE): may chu van song
    for (int i = 0; i < 20; ++i) {
        const int fd = connectTo(port);
        const std::string request = get("/api/products?limit=100");
        send(fd, request.data(), request.size(), MSG_NOSIGNAL);
        close(fd);
    }
    CHECK(roundTrip(port, get("/api/health")).status == 200);

    // 8) 60 khach dong thoi
    {
        std::atomic<int> ok{0};
        std::vector<std::thread> clients;
        for (int i = 0; i < 60; ++i) {
            clients.emplace_back([&] {
                if (roundTrip(port, get("/api/products?limit=30")).status == 200) ++ok;
            });
        }
        for (auto& c : clients) c.join();
        CHECK(ok.load() == 60);
    }

    stop = true;
    server.join();
    CHECK(serverCode.load() == 0);
    std::error_code ignored;
    fs::remove_all(temp, ignored);
    std::cout << "test_http: " << (gChecks - gFailures) << "/" << gChecks << " kiem tra dat\n";
    return gFailures == 0 ? 0 : 1;
}

#endif
