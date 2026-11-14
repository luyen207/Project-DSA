// Ngu canh dung chung cua REST API: du lieu, phien dang nhap, gioi han toc do, Router.
#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "CartEngine.h"
#include "Support.h"
#include "http/HttpTypes.h"
#include "storage/ActivityLog.h"
#include "storage/JsonStore.h"

namespace shoppro {

enum class Access { Public, Customer, Admin };

struct Session {
    std::string userId;
    std::string role;
    std::int64_t expiresAt = 0;
};

// Mot yeu cau da duoc Router phan tich. Thong tin nguoi dung luu BAN SAO (chuoi),
// tuyet doi khong giu tham chieu vao ctx.data (data bi thay the sau moi lan commit).
struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> query;
    std::map<std::string, std::string> params;
    Json body = Json::object();
    std::string ip;
    std::string token;
    std::string userId;
    std::string role;
    std::string userName;
    std::string userEmail;
};

// Chong do mat khau: dem lan that bai theo khoa (ip|email), khoa tam thoi khi vuot nguong
class Throttle {
public:
    int blockedFor(const std::string& key);   // so giay con bi khoa, 0 neu khong khoa
    void recordFailure(const std::string& key, int maxFailures, int windowSeconds, int blockSeconds);
    void clear(const std::string& key);

private:
    struct Entry {
        int failures = 0;
        std::int64_t windowStart = 0;
        std::int64_t blockedUntil = 0;
    };
    std::mutex mutex_;
    std::map<std::string, Entry> entries_;
};

struct Ctx {
    Ctx(JsonStore& jsonStore, ActivityLog& activityLog, Json initial)
        : data(std::move(initial)), store(jsonStore), log(activityLog) {}

    std::mutex mu;                       // bao ve data, carts, sessions
    Json data;
    JsonStore& store;
    ActivityLog& log;
    CartEngine carts;
    std::map<std::string, Session> sessions;
    Throttle throttle;

    // Ghi `next` xuong dia roi moi thay vao `data`. Ghi loi thi nem exception va `data` khong doi.
    // SAU commit, moi con tro/tham chieu vao `data` cu deu khong con hop le.
    void commit(Json&& next);

    void audit(const Request& req, const std::string& action, const std::string& targetType,
               const std::string& targetId, const std::string& result, const Json& details = Json::object());
    void auditAs(const std::string& userId, const std::string& role, const std::string& ip,
                 const std::string& action, const std::string& targetType, const std::string& targetId,
                 const std::string& result, const Json& details = Json::object());
};

// Ham xu ly: chay khi dang giu ctx.mu (tru route selfLocking: tu khoa trong ham)
using Handler = std::function<HttpResponse(Ctx&, Request&)>;

class Router {
public:
    // pattern vi du: "/api/products/:id". selfLocking: ham tu quan ly mutex (chi dung cho route Public)
    void add(const std::string& method, const std::string& pattern, Access access, Handler handler,
             bool selfLocking = false);

    HttpResponse dispatch(Ctx& ctx, const HttpRequest& request) const;

private:
    struct Route {
        std::string method;
        std::vector<std::string> segments;
        Access access;
        Handler handler;
        bool selfLocking;
    };
    std::vector<Route> routes_;
};

// ---- dang ky route theo nhom (moi nhom 1 tep trong routes/) ----
void registerPublicRoutes(Router& router);    // health, products, categories, ai chat
void registerAuthRoutes(Router& router);      // register, login, logout, me, profile
void registerCartRoutes(Router& router);
void registerOrderRoutes(Router& router);
void registerAdminCatalogRoutes(Router& router);
void registerAdminOpsRoutes(Router& router);

inline Router buildRouter() {
    Router router;
    registerPublicRoutes(router);
    registerAuthRoutes(router);
    registerCartRoutes(router);
    registerOrderRoutes(router);
    registerAdminCatalogRoutes(router);
    registerAdminOpsRoutes(router);
    return router;
}

}  // namespace shoppro
