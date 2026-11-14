#include <algorithm>
#include <iostream>

#include "ApiContext.h"

namespace shoppro {

// ------------------------------- Throttle -------------------------------
int Throttle::blockedFor(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto found = entries_.find(key);
    if (found == entries_.end()) return 0;
    const std::int64_t now = timeutil::epochSeconds();
    if (found->second.blockedUntil > now) return static_cast<int>(found->second.blockedUntil - now);
    return 0;
}

void Throttle::recordFailure(const std::string& key, int maxFailures, int windowSeconds, int blockSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    const std::int64_t now = timeutil::epochSeconds();
    if (entries_.size() > 20000) {   // don dep de khong phinh bo nho
        for (auto it = entries_.begin(); it != entries_.end();) {
            if (it->second.blockedUntil <= now && now - it->second.windowStart > windowSeconds) it = entries_.erase(it);
            else ++it;
        }
    }
    Entry& entry = entries_[key];
    if (now - entry.windowStart > windowSeconds) {
        entry.failures = 0;
        entry.windowStart = now;
    }
    if (++entry.failures >= maxFailures) {
        entry.blockedUntil = now + blockSeconds;
        entry.failures = 0;
        entry.windowStart = now;
    }
}

void Throttle::clear(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.erase(key);
}

// --------------------------------- Ctx ---------------------------------
void Ctx::commit(Json&& next) {
    store.persist(next);        // loi -> exception, data giu nguyen
    data = std::move(next);     // sau dong nay moi tham chieu vao data cu la treo
}

void Ctx::auditAs(const std::string& userId, const std::string& role, const std::string& ip,
                  const std::string& action, const std::string& targetType, const std::string& targetId,
                  const std::string& result, const Json& details) {
    log.record(action, userId, role, targetType, targetId, result, details, ip);
}

void Ctx::audit(const Request& req, const std::string& action, const std::string& targetType,
                const std::string& targetId, const std::string& result, const Json& details) {
    auditAs(req.userId, req.role, req.ip, action, targetType, targetId, result, details);
}

// -------------------------------- Router --------------------------------
namespace {

std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> parts;
    for (const auto& piece : text::split(path, '/')) {
        if (!piece.empty()) parts.push_back(piece);
    }
    return parts;
}

}  // namespace

void Router::add(const std::string& method, const std::string& pattern, Access access, Handler handler,
                 bool selfLocking) {
    routes_.push_back({method, splitPath(pattern), access, std::move(handler), selfLocking});
}

HttpResponse Router::dispatch(Ctx& ctx, const HttpRequest& http) const {
    try {
        // ---- 1. Phan tich duong dan + query ----
        std::string path = http.target;
        std::string queryString;
        const auto question = path.find('?');
        if (question != std::string::npos) {
            queryString = path.substr(question + 1);
            path.resize(question);
        }
        Request req;
        req.method = http.method;
        req.ip = http.ip;
        std::vector<std::string> segments;
        for (const auto& piece : splitPath(path)) segments.push_back(text::urlDecode(piece));
        for (const auto& pair : text::split(queryString, '&')) {
            if (pair.empty()) continue;
            const auto equals = pair.find('=');
            const std::string key = text::urlDecode(pair.substr(0, equals));
            const std::string value = equals == std::string::npos ? "" : text::urlDecode(pair.substr(equals + 1));
            if (!text::validUtf8(key) || !text::validUtf8(value)) throw ApiError(400, "Tham số truy vấn không phải UTF-8.");
            req.query[key] = value;
        }
        req.path = "/" + (segments.empty() ? std::string() : segments.front());
        for (std::size_t i = 1; i < segments.size(); ++i) req.path += "/" + segments[i];

        // ---- 2. Tim route ----
        const Route* matched = nullptr;
        std::vector<std::string> allowed;
        for (const auto& route : routes_) {
            if (route.segments.size() != segments.size()) continue;
            std::map<std::string, std::string> params;
            bool fits = true;
            for (std::size_t i = 0; i < segments.size() && fits; ++i) {
                const std::string& pattern = route.segments[i];
                if (!pattern.empty() && pattern[0] == ':') params[pattern.substr(1)] = segments[i];
                else fits = pattern == segments[i];
            }
            if (!fits) continue;
            if (route.method == http.method) {
                matched = &route;
                req.params = std::move(params);
                break;
            }
            allowed.push_back(route.method);
        }
        if (!matched) {
            if (!allowed.empty()) {
                std::sort(allowed.begin(), allowed.end());
                std::string header;
                for (const auto& method : allowed) header += (header.empty() ? "" : ", ") + method;
                return {405, {{"error", "Phương thức không được hỗ trợ cho đường dẫn này."}}, {{"Allow", header}}};
            }
            throw ApiError(404, "Không tìm thấy đường dẫn.");
        }

        // ---- 3. Noi dung yeu cau ----
        if (http.method == "POST" || http.method == "PUT" || http.method == "PATCH" || http.method == "DELETE") {
            req.body = jsonx::parseObject(http.body);
        }

        // ---- 4. Khoa du lieu + xac thuc ----
        std::unique_lock<std::mutex> lock(ctx.mu, std::defer_lock);
        if (!matched->selfLocking) lock.lock();

        if (matched->access != Access::Public) {
            const auto header = http.headers.find("authorization");
            if (header == http.headers.end() || header->second.rfind("Bearer ", 0) != 0) {
                throw ApiError(401, "Bạn cần đăng nhập để tiếp tục.");
            }
            req.token = header->second.substr(7);
            const auto session = ctx.sessions.find(req.token);
            if (session == ctx.sessions.end()) throw ApiError(401, "Phiên đăng nhập không hợp lệ. Vui lòng đăng nhập lại.");
            if (session->second.expiresAt <= timeutil::epochSeconds()) {
                ctx.sessions.erase(session);
                throw ApiError(401, "Phiên đăng nhập đã hết hạn. Vui lòng đăng nhập lại.");
            }
            const std::string wantedRole = matched->access == Access::Admin ? "admin" : "customer";
            const Json* user = jsonx::findByStringId(ctx.data.at("users"), session->second.userId);
            if (!user) {
                ctx.sessions.erase(session);
                throw ApiError(401, "Tài khoản không còn tồn tại.");
            }
            if (user->value("role", "") != wantedRole || session->second.role != wantedRole) {
                throw ApiError(403, "Bạn không có quyền truy cập chức năng này.");
            }
            if (user->value("status", "active") != "active") throw ApiError(403, "Tài khoản này đã bị khóa.");
            req.userId = user->value("id", "");
            req.role = wantedRole;
            req.userName = user->value("name", "");
            req.userEmail = user->value("email", "");
        }

        return matched->handler(ctx, req);
    } catch (const ApiError& error) {
        HttpResponse response{error.status, {{"error", error.what()}}, {}};
        if (error.retryAfterSeconds > 0) response.headers["Retry-After"] = std::to_string(error.retryAfterSeconds);
        return response;
    } catch (const Json::exception&) {
        return {400, {{"error", "Dữ liệu gửi lên không hợp lệ hoặc chưa đầy đủ."}}, {}};
    } catch (const std::exception& error) {
        std::cerr << "[loi] " << http.method << ' ' << http.target << ": " << error.what() << '\n';
        return {500, {{"error", "Không thể hoàn tất yêu cầu. Vui lòng thử lại."}}, {}};
    }
}

}  // namespace shoppro
