// Dang ky / dang nhap / dang xuat / ho so. Bam mat khau PBKDF2-HMAC-SHA256 (OpenSSL).
// Cac route dang nhap/dang ky tu khoa mutex: viec bam ton ~0.1-0.3s KHONG duoc chan cac yeu cau khac.
#include <algorithm>
#include <cctype>

#include "Presenters.h"

namespace shoppro {
namespace {

constexpr std::int64_t kSessionSeconds = 8 * 60 * 60;
constexpr std::size_t kMaxSessions = 10000;

std::string makeUserId(const std::string& prefix) { return prefix + "-" + crypto::randomHex(6); }

std::string passwordField(const Json& body) {
    if (!body.contains("password") || !body["password"].is_string()) throw ApiError(400, "Thiếu trường 'password'.");
    const std::string password = body["password"].get<std::string>();
    if (password.empty() || password.size() > 128) throw ApiError(400, "Mật khẩu không hợp lệ (tối đa 128 ký tự).");
    return password;
}

// Tao phien (ctx.mu dang duoc giu)
Json issueSession(Ctx& ctx, const Json& user) {
    const std::int64_t now = timeutil::epochSeconds();
    for (auto it = ctx.sessions.begin(); it != ctx.sessions.end();) {
        if (it->second.expiresAt <= now) it = ctx.sessions.erase(it);
        else ++it;
    }
    while (ctx.sessions.size() >= kMaxSessions) {   // het cho: bo phien sap het han nhat
        auto oldest = std::min_element(ctx.sessions.begin(), ctx.sessions.end(), [](const auto& a, const auto& b) {
            return a.second.expiresAt < b.second.expiresAt;
        });
        ctx.sessions.erase(oldest);
    }
    const std::string token = crypto::randomHex(32);
    ctx.sessions[token] = {user.value("id", ""), user.value("role", ""), now + kSessionSeconds};
    const bool admin = user.value("role", "") == "admin";
    return {{"user", admin ? view::adminUser(user) : view::publicUser(user)},
            {"token", token},
            {"expiresAt", timeutil::toIso(now + kSessionSeconds)}};
}

HttpResponse registerCustomer(Ctx& ctx, Request& req) {
    const std::string name = jsonx::requiredString(req.body, "name", 80);
    const std::string email = text::lower(jsonx::requiredString(req.body, "email", 160));
    const std::string phone = jsonx::requiredString(req.body, "phone", 40);
    const std::string password = passwordField(req.body);
    if (name.size() < 2) throw ApiError(400, "Họ tên cần có ít nhất 2 ký tự.");
    if (!text::validEmail(email)) throw ApiError(400, "Email không hợp lệ.");
    if (!text::validPhone(phone)) throw ApiError(400, "Số điện thoại không hợp lệ.");
    if (password.size() < 8) throw ApiError(400, "Mật khẩu cần có ít nhất 8 ký tự.");
    if (password.find_first_of("0123456789") == std::string::npos ||
        std::none_of(password.begin(), password.end(), [](unsigned char c) { return std::isalpha(c) != 0; })) {
        throw ApiError(400, "Mật khẩu cần có cả chữ và số.");
    }

    const std::string throttleKey = "register|" + req.ip;
    if (const int wait = ctx.throttle.blockedFor(throttleKey)) {
        ctx.auditAs("", "anonymous", req.ip, "auth.register.blocked", "user", "", "blocked", {{"email", email}});
        throw ApiError(429, "Bạn đăng ký quá nhiều lần. Vui lòng thử lại sau.", wait);
    }
    ctx.throttle.recordFailure(throttleKey, 20, 3600, 3600);   // toi da 20 lan dang ky / gio / IP

    const std::string salt = crypto::randomHex(16);
    const std::string hash = crypto::hashPassword(password, salt, kPasswordIterations);   // ngoai mutex

    std::lock_guard<std::mutex> lock(ctx.mu);
    for (const auto& user : ctx.data.at("users")) {
        if (text::lower(user.value("email", "")) == email) throw ApiError(409, "Email này đã được đăng ký.");
    }
    Json next = ctx.data;
    const std::string id = makeUserId("cus");
    Json user{{"id", id}, {"name", name}, {"email", email}, {"phone", phone}, {"address", ""},
              {"role", "customer"}, {"status", "active"}, {"joinedAt", timeutil::todayIso()},
              {"passwordSalt", salt}, {"passwordHash", hash}, {"passwordIterations", kPasswordIterations}};
    next["users"].push_back(user);
    next["carts"][id] = Json::array();
    ctx.commit(std::move(next));
    ctx.auditAs(id, "customer", req.ip, "auth.registered", "user", id, "success");
    return {201, issueSession(ctx, user), {}};
}

Handler loginFor(const std::string& role) {
    return [role](Ctx& ctx, Request& req) -> HttpResponse {
        const std::string email = text::lower(jsonx::requiredString(req.body, "email", 160));
        const std::string password = passwordField(req.body);
        const std::string actorRole = role;

        const std::string pairKey = "login|" + req.ip + "|" + email;
        const std::string ipKey = "login-ip|" + req.ip;
        int wait = std::max(ctx.throttle.blockedFor(pairKey), ctx.throttle.blockedFor(ipKey));
        if (wait > 0) {
            ctx.auditAs("", actorRole, req.ip, "auth.login.blocked", "user", "", "blocked", {{"email", email}});
            throw ApiError(429, "Đăng nhập sai quá nhiều lần. Vui lòng thử lại sau.", wait);
        }

        // 1) Lay ban sao ban ghi nguoi dung duoi khoa (nhanh)
        Json candidate;
        {
            std::lock_guard<std::mutex> lock(ctx.mu);
            for (const auto& user : ctx.data.at("users")) {
                if (text::lower(user.value("email", "")) == email && user.value("role", "") == role) {
                    candidate = user;
                    break;
                }
            }
        }
        // 2) Bam mat khau NGOAI khoa. Khong co user van bam gia de thoi gian phan hoi dong deu.
        bool passwordOk;
        if (candidate.is_object()) {
            passwordOk = crypto::verifyPassword(password, candidate);
        } else {
            static const std::string dummySalt = crypto::randomHex(16);
            crypto::hashPassword(password, dummySalt, kPasswordIterations);
            passwordOk = false;
        }
        if (!passwordOk) {
            ctx.throttle.recordFailure(pairKey, 5, 900, 900);
            ctx.throttle.recordFailure(ipKey, 40, 900, 900);
            ctx.auditAs(candidate.is_object() ? candidate.value("id", "") : "", actorRole, req.ip,
                        "auth.login.failed", "user", candidate.is_object() ? candidate.value("id", "") : "",
                        "failure", {{"email", email}});
            throw ApiError(401, "Email hoặc mật khẩu chưa chính xác.");
        }

        // 3) Mo phien duoi khoa, kiem tra lai trang thai tai khoan
        std::lock_guard<std::mutex> lock(ctx.mu);
        const Json* current = jsonx::findByStringId(ctx.data.at("users"), candidate.value("id", ""));
        if (!current || current->value("status", "active") != "active") {
            ctx.auditAs(candidate.value("id", ""), actorRole, req.ip, "auth.login.blocked", "user",
                        candidate.value("id", ""), "blocked", {{"reason", "locked"}});
            throw ApiError(403, "Tài khoản này đã bị khóa.");
        }
        ctx.throttle.clear(pairKey);
        Json response = issueSession(ctx, *current);
        ctx.auditAs(current->value("id", ""), actorRole, req.ip, "auth.login.succeeded", "user",
                    current->value("id", ""), "success");
        return {200, response, {}};
    };
}

Handler logoutFor(const std::string&) {
    return [](Ctx& ctx, Request& req) -> HttpResponse {
        ctx.sessions.erase(req.token);
        ctx.audit(req, "auth.logout", "user", req.userId, "success");
        return {200, {{"message", "Đã đăng xuất."}}, {}};
    };
}

HttpResponse meCustomer(Ctx& ctx, Request& req) {
    const Json* user = jsonx::findByStringId(ctx.data.at("users"), req.userId);
    if (!user) throw ApiError(404, "Không tìm thấy tài khoản.");
    return {200, view::publicUser(*user), {}};
}

HttpResponse meAdmin(Ctx& ctx, Request& req) {
    const Json* user = jsonx::findByStringId(ctx.data.at("users"), req.userId);
    if (!user) throw ApiError(404, "Không tìm thấy tài khoản.");
    return {200, view::adminUser(*user), {}};
}

HttpResponse updateProfile(Ctx& ctx, Request& req) {
    Json next = ctx.data;
    Json* user = jsonx::findByStringId(next["users"], req.userId);
    if (!user) throw ApiError(404, "Không tìm thấy tài khoản.");
    Json changed = Json::array();
    if (req.body.contains("name")) {
        const std::string name = jsonx::requiredString(req.body, "name", 80);
        if (name.size() < 2) throw ApiError(400, "Họ tên cần có ít nhất 2 ký tự.");
        (*user)["name"] = name;
        changed.push_back("name");
    }
    if (req.body.contains("phone")) {
        const std::string phone = jsonx::requiredString(req.body, "phone", 40);
        if (!text::validPhone(phone)) throw ApiError(400, "Số điện thoại không hợp lệ.");
        (*user)["phone"] = phone;
        changed.push_back("phone");
    }
    if (req.body.contains("address")) {
        (*user)["address"] = jsonx::optionalString(req.body, "address", 300);
        changed.push_back("address");
    }
    if (changed.empty()) throw ApiError(400, "Không có thông tin nào để cập nhật.");
    const Json result = view::publicUser(*user);
    ctx.commit(std::move(next));
    ctx.audit(req, "customer.profile.updated", "user", req.userId, "success", {{"fields", changed}});
    return {200, result, {}};
}

}  // namespace

void registerAuthRoutes(Router& router) {
    router.add("POST", "/api/auth/register", Access::Public, registerCustomer, true);
    router.add("POST", "/api/auth/login", Access::Public, loginFor("customer"), true);
    router.add("POST", "/api/admin/auth/login", Access::Public, loginFor("admin"), true);
    router.add("POST", "/api/auth/logout", Access::Customer, logoutFor("customer"));
    router.add("POST", "/api/admin/auth/logout", Access::Admin, logoutFor("admin"));
    router.add("GET", "/api/auth/me", Access::Customer, meCustomer);
    router.add("GET", "/api/admin/auth/me", Access::Admin, meAdmin);
    router.add("PATCH", "/api/users/me", Access::Customer, updateProfile);
}

}  // namespace shoppro
