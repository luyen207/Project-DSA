// Quan tri van hanh: dashboard, bao cao, don hang, khach hang, cai dat, nhat ky hoat dong.
#include <algorithm>
#include <map>
#include <set>

#include "Presenters.h"

namespace shoppro {
namespace {

std::string dayOf(const Json& order) { return order.value("createdAt", "").substr(0, 10); }

bool validDate(const std::string& date) {
    return date.size() == 10 && date[4] == '-' && date[7] == '-' && timeutil::parseIso(date) >= 0 &&
           timeutil::toIso(timeutil::parseIso(date)).substr(0, 10) == date;
}

std::vector<const Json*> sortedOrders(const Json& orders) {
    std::vector<const Json*> list;
    for (const auto& order : orders) list.push_back(&order);
    std::stable_sort(list.begin(), list.end(), [](const Json* a, const Json* b) {
        return a->value("createdAt", "") > b->value("createdAt", "");
    });
    return list;
}

// Doanh thu duoc ghi nhan khi don o trang thai "completed"
std::int64_t recognisedRevenue(const Json& order) {
    return order.value("status", "") == "completed" ? jsonx::integer(order, "total") : 0;
}

Json bestSellers(const Json& orders, std::size_t limit) {
    struct Seller { std::string name, image; std::int64_t quantity = 0, revenue = 0; };
    std::map<std::int64_t, Seller> sellers;
    for (const auto& order : orders) {
        if (order.value("status", "") == "cancelled") continue;
        for (const auto& item : order.value("items", Json::array())) {
            Seller& seller = sellers[jsonx::integer(item, "productId")];
            seller.name = item.value("productName", "");
            seller.image = item.value("image", "");
            seller.quantity += jsonx::integer(item, "quantity");
            seller.revenue = view::checkedAdd(seller.revenue, view::checkedMultiply(jsonx::integer(item, "unitPrice"), jsonx::integer(item, "quantity")));
        }
    }
    std::vector<std::pair<std::int64_t, Seller>> ranked(sellers.begin(), sellers.end());
    std::stable_sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.second.quantity > b.second.quantity; });
    Json result = Json::array();
    for (std::size_t i = 0; i < ranked.size() && i < limit; ++i) {
        result.push_back({{"productId", ranked[i].first}, {"name", ranked[i].second.name},
                          {"image", ranked[i].second.image}, {"quantity", ranked[i].second.quantity},
                          {"revenue", ranked[i].second.revenue}});
    }
    return result;
}

Json statusSeries(const Json& orders) {
    std::map<std::string, std::int64_t> counts{{"pending", 0}, {"confirmed", 0}, {"shipping", 0}, {"completed", 0}, {"cancelled", 0}};
    for (const auto& order : orders) ++counts[order.value("status", "pending")];
    Json result = Json::array();
    for (const char* status : {"pending", "confirmed", "shipping", "completed", "cancelled"}) {
        result.push_back({{"status", status}, {"value", counts[status]}});
    }
    return result;
}

// Chuoi doanh thu theo ngay (dien du ngay khong co don) tu from den to (YYYY-MM-DD)
Json dailySeries(const Json& orders, const std::string& from, const std::string& to) {
    std::map<std::string, std::pair<std::int64_t, std::int64_t>> byDay;   // ngay -> {doanh thu, so don}
    for (const auto& order : orders) {
        if (order.value("status", "") == "cancelled") continue;
        auto& cell = byDay[dayOf(order)];
        cell.first += recognisedRevenue(order);
        cell.second += 1;
    }
    Json series = Json::array();
    for (std::int64_t t = timeutil::parseIso(from); t <= timeutil::parseIso(to); t += 86400) {
        const std::string day = timeutil::toIso(t).substr(0, 10);
        const auto found = byDay.find(day);
        series.push_back({{"date", day}, {"label", day.substr(8, 2) + "/" + day.substr(5, 2)},
                          {"revenue", found == byDay.end() ? 0 : found->second.first},
                          {"orders", found == byDay.end() ? 0 : found->second.second}});
    }
    return series;
}

HttpResponse dashboard(Ctx& ctx, Request&) {
    const Json& data = ctx.data;
    const Json& orders = data.at("orders");
    std::int64_t revenue = 0, pending = 0, todayRevenue = 0, todayOrders = 0;
    const std::string today = timeutil::todayIso();
    for (const auto& order : orders) {
        revenue = view::checkedAdd(revenue, recognisedRevenue(order));
        if (order.value("status", "") == "pending") ++pending;
        if (dayOf(order) == today && order.value("status", "") != "cancelled") {
            ++todayOrders;
            todayRevenue += recognisedRevenue(order);
        }
    }
    std::int64_t customers = 0;
    for (const auto& user : data.at("users")) {
        if (user.value("role", "") == "customer") ++customers;
    }
    const std::int64_t threshold = jsonx::integer(data.at("settings"), "lowStockThreshold", 10);
    std::vector<const Json*> low;
    std::int64_t out = 0;
    for (const auto& product : data.at("products")) {
        const std::int64_t stock = jsonx::integer(product, "stock");
        if (stock == 0) ++out;
        if (stock <= threshold) low.push_back(&product);
    }
    std::stable_sort(low.begin(), low.end(), [](const Json* a, const Json* b) { return jsonx::integer(*a, "stock") < jsonx::integer(*b, "stock"); });
    Json lowStock = Json::array();
    for (std::size_t i = 0; i < low.size() && i < 8; ++i) {
        lowStock.push_back({{"id", jsonx::integer(*low[i], "id")}, {"name", low[i]->value("name", "")},
                            {"image", low[i]->value("image", "")}, {"stock", jsonx::integer(*low[i], "stock")}});
    }
    Json recent = Json::array();
    const auto ordered = sortedOrders(orders);
    for (std::size_t i = 0; i < ordered.size() && i < 6; ++i) recent.push_back(view::adminOrder(*ordered[i]));

    const std::int64_t now = timeutil::epochSeconds();
    const std::string from = timeutil::toIso(now - 13 * 86400).substr(0, 10);
    return {200,
            {{"productCount", data.at("products").size()},
             {"orderCount", orders.size()},
             {"customerCount", customers},
             {"revenue", revenue},
             {"pendingOrders", pending},
             {"todayOrders", todayOrders},
             {"todayRevenue", todayRevenue},
             {"lowStockThreshold", threshold},
             {"lowStockCount", low.size()},
             {"outOfStockCount", out},
             {"revenueSeries", dailySeries(orders, from, today)},
             {"statusSeries", statusSeries(orders)},
             {"recentOrders", recent},
             {"bestSellers", bestSellers(orders, 5)},
             {"lowStock", lowStock}},
            {}};
}

HttpResponse reportSummary(Ctx& ctx, Request& req) {
    const std::string today = timeutil::todayIso();
    const std::string to = req.query.count("to") && !req.query.at("to").empty() ? req.query.at("to") : today;
    const std::string from = req.query.count("from") && !req.query.at("from").empty()
                                 ? req.query.at("from")
                                 : timeutil::toIso(timeutil::parseIso(to) - 29 * 86400).substr(0, 10);
    if (!validDate(from) || !validDate(to) || from > to) throw ApiError(400, "Khoảng ngày báo cáo không hợp lệ (YYYY-MM-DD, từ ≤ đến).");
    if (timeutil::parseIso(to) - timeutil::parseIso(from) > 366 * 86400) throw ApiError(400, "Khoảng báo cáo tối đa 366 ngày.");

    const Json& data = ctx.data;
    Json inRange = Json::array();
    for (const auto& order : data.at("orders")) {
        if (dayOf(order) >= from && dayOf(order) <= to) inRange.push_back(order);
    }

    std::int64_t revenue = 0;
    std::int64_t counted = 0;
    std::set<std::string> customers;
    std::set<std::int64_t> products;
    std::map<std::string, std::pair<std::int64_t, std::int64_t>> byCategory;   // ten -> {so luong, doanh thu}
    std::map<std::string, std::pair<std::string, std::int64_t>> byCustomer;    // id -> {ten, chi tieu}
    for (const auto& order : inRange) {
        if (order.value("status", "") == "cancelled") continue;
        ++counted;
        customers.insert(order.value("customerId", ""));
        const std::int64_t recognised = recognisedRevenue(order);
        revenue = view::checkedAdd(revenue, recognised);
        auto& customer = byCustomer[order.value("customerId", "")];
        customer.first = order.value("customerName", "");
        customer.second += recognised;
        for (const auto& item : order.value("items", Json::array())) {
            const std::int64_t productId = jsonx::integer(item, "productId");
            products.insert(productId);
            const Json* product = view::findProduct(data, productId);
            const Json* category = product ? view::findCategory(data, jsonx::integer(*product, "categoryId")) : nullptr;
            auto& cell = byCategory[category ? category->value("name", "Khác") : "Khác"];
            cell.first += jsonx::integer(item, "quantity");
            cell.second += order.value("status", "") == "completed"
                               ? view::checkedMultiply(jsonx::integer(item, "unitPrice"), jsonx::integer(item, "quantity"))
                               : 0;
        }
    }
    Json categorySeries = Json::array();
    for (const auto& entry : byCategory) categorySeries.push_back({{"name", entry.first}, {"value", entry.second.first}, {"revenue", entry.second.second}});
    std::vector<std::pair<std::string, std::pair<std::string, std::int64_t>>> ranked(byCustomer.begin(), byCustomer.end());
    std::stable_sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.second.second > b.second.second; });
    Json topCustomers = Json::array();
    for (std::size_t i = 0; i < ranked.size() && i < 5; ++i) {
        topCustomers.push_back({{"customerId", ranked[i].first}, {"name", ranked[i].second.first}, {"spent", ranked[i].second.second}});
    }
    std::int64_t completedCount = 0;
    for (const auto& order : inRange) if (order.value("status", "") == "completed") ++completedCount;

    return {200,
            {{"from", from}, {"to", to},
             {"revenue", revenue},
             {"orderCount", counted},
             {"completedCount", completedCount},
             {"averageOrderValue", completedCount ? revenue / completedCount : 0},
             {"customerCount", customers.size()},
             {"productCount", products.size()},
             {"revenueSeries", dailySeries(inRange, from, to)},
             {"categorySeries", categorySeries},
             {"statusSeries", statusSeries(inRange)},
             {"topProducts", bestSellers(inRange, 8)},
             {"topCustomers", topCustomers}},
            {}};
}

// ------------------------------- Don hang -------------------------------
HttpResponse listOrders(Ctx& ctx, Request& req) {
    const std::string status = req.query.count("status") ? req.query.at("status") : "";
    const std::string q = text::searchKey(req.query.count("q") ? req.query.at("q") : "");
    Json result = Json::array();
    for (const Json* order : sortedOrders(ctx.data.at("orders"))) {
        if (!status.empty() && order->value("status", "") != status) continue;
        if (!q.empty()) {
            const std::string hay = text::searchKey(order->value("id", "") + " " + order->value("customerName", "") + " " +
                                                    order->value("email", "") + " " + order->value("phone", ""));
            if (hay.find(q) == std::string::npos) continue;
        }
        result.push_back(view::adminOrder(*order));
    }
    return {200, result, {}};
}

HttpResponse getOrder(Ctx& ctx, Request& req) {
    const Json* order = jsonx::findByStringId(ctx.data.at("orders"), req.params.at("id"));
    if (!order) throw ApiError(404, "Không tìm thấy đơn hàng.");
    return {200, view::adminOrder(*order), {}};
}

HttpResponse updateOrderStatus(Ctx& ctx, Request& req) {
    const std::string id = req.params.at("id");
    const std::string status = jsonx::requiredString(req.body, "status", 20);
    static const std::set<std::string> valid{"pending", "confirmed", "shipping", "completed", "cancelled"};
    if (!valid.count(status)) throw ApiError(400, "Trạng thái đơn hàng không hợp lệ.");
    Json next = ctx.data;
    Json* order = jsonx::findByStringId(next["orders"], id);
    if (!order) throw ApiError(404, "Không tìm thấy đơn hàng.");
    const std::string before = order->value("status", "");
    if (!view::allowedTransition(before, status)) {
        ctx.audit(req, "admin.order.status_changed", "order", id, "failure", {{"from", before}, {"to", status}});
        throw ApiError(409, "Không thể chuyển đơn từ \"" + before + "\" sang \"" + status + "\".");
    }
    if (status == "cancelled") view::restoreStockOnCancel(next, *order);
    (*order)["status"] = status;
    (*order)["updatedAt"] = timeutil::nowIso();
    const Json response = view::adminOrder(*order);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.order.status_changed", "order", id, "success", {{"from", before}, {"to", status}});
    return {200, response, {}};
}

// ------------------------------- Khach hang -------------------------------
HttpResponse listCustomers(Ctx& ctx, Request&) {
    Json result = Json::array();
    for (const auto& user : ctx.data.at("users")) {
        if (user.value("role", "") == "customer") result.push_back(view::adminCustomer(ctx.data, user));
    }
    return {200, result, {}};
}

HttpResponse customerOrders(Ctx& ctx, Request& req) {
    const Json* user = jsonx::findByStringId(ctx.data.at("users"), req.params.at("id"));
    if (!user || user->value("role", "") != "customer") throw ApiError(404, "Không tìm thấy khách hàng.");
    Json result = Json::array();
    for (const Json* order : sortedOrders(ctx.data.at("orders"))) {
        if (order->value("customerId", "") == req.params.at("id")) result.push_back(view::adminOrder(*order));
    }
    return {200, result, {}};
}

HttpResponse updateCustomerStatus(Ctx& ctx, Request& req) {
    const std::string id = req.params.at("id");
    const std::string status = jsonx::requiredString(req.body, "status", 20);
    if (status != "active" && status != "locked") throw ApiError(400, "Trạng thái khách hàng phải là active hoặc locked.");
    Json next = ctx.data;
    Json* user = jsonx::findByStringId(next["users"], id);
    if (!user || user->value("role", "") != "customer") throw ApiError(404, "Không tìm thấy khách hàng.");
    (*user)["status"] = status;
    const Json response = view::adminCustomer(next, *user);
    ctx.commit(std::move(next));
    if (status == "locked") {   // thu hoi moi phien dang nhap cua khach
        for (auto it = ctx.sessions.begin(); it != ctx.sessions.end();) {
            if (it->second.userId == id) it = ctx.sessions.erase(it);
            else ++it;
        }
    }
    ctx.audit(req, "admin.customer.status_changed", "user", id, "success", {{"status", status}});
    return {200, response, {}};
}

// --------------------------------- Cai dat ---------------------------------
HttpResponse getSettings(Ctx& ctx, Request&) { return {200, ctx.data.at("settings"), {}}; }

HttpResponse putSettings(Ctx& ctx, Request& req) {
    const std::string storeName = jsonx::requiredString(req.body, "storeName", 100);
    const std::string email = jsonx::requiredString(req.body, "supportEmail", 160);
    const std::string phone = jsonx::requiredString(req.body, "supportPhone", 40);
    const std::string currency = jsonx::requiredString(req.body, "currency", 3);
    const std::int64_t threshold = jsonx::integer(req.body, "lowStockThreshold", -1);
    if (!text::validEmail(email)) throw ApiError(400, "Email hỗ trợ không hợp lệ.");
    if (threshold < 0 || threshold > kMaxStock) throw ApiError(400, "Ngưỡng cảnh báo tồn kho phải từ 0 đến " + std::to_string(kMaxStock) + ".");
    if (currency != "VND" && currency != "USD") throw ApiError(400, "Tiền tệ chỉ hỗ trợ VND hoặc USD.");
    Json next = ctx.data;
    next["settings"] = {{"storeName", storeName}, {"supportEmail", email}, {"supportPhone", phone},
                        {"lowStockThreshold", threshold}, {"currency", currency}};
    const Json response = next["settings"];
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.settings.updated", "settings", "store", "success");
    return {200, response, {}};
}

// ----------------------------- Nhat ky hoat dong -----------------------------
HttpResponse activityLogs(Ctx& ctx, Request& req) {
    LogQuery query;
    query.page = static_cast<int>(std::clamp<std::int64_t>(jsonx::queryInteger(req.query, "page", 1), 1, 1000000));
    query.limit = static_cast<int>(std::clamp<std::int64_t>(jsonx::queryInteger(req.query, "limit", 50), 1, 200));
    const auto field = [&](const char* key) { return req.query.count(key) ? req.query.at(key) : std::string(); };
    query.action = field("action");
    query.actorId = field("actorId");
    query.result = field("result");
    query.from = field("from");
    query.to = field("to");
    if ((!query.from.empty() && !validDate(query.from)) || (!query.to.empty() && !validDate(query.to))) {
        throw ApiError(400, "Ngày lọc phải theo định dạng YYYY-MM-DD.");
    }
    return {200, ctx.log.query(query), {}};
}

}  // namespace

void registerAdminOpsRoutes(Router& router) {
    router.add("GET", "/api/admin/dashboard", Access::Admin, dashboard);
    router.add("GET", "/api/admin/reports/summary", Access::Admin, reportSummary);
    router.add("GET", "/api/admin/orders", Access::Admin, listOrders);
    router.add("GET", "/api/admin/orders/:id", Access::Admin, getOrder);
    router.add("PATCH", "/api/admin/orders/:id/status", Access::Admin, updateOrderStatus);
    router.add("GET", "/api/admin/customers", Access::Admin, listCustomers);
    router.add("GET", "/api/admin/customers/:id/orders", Access::Admin, customerOrders);
    router.add("PATCH", "/api/admin/customers/:id/status", Access::Admin, updateCustomerStatus);
    router.add("GET", "/api/admin/settings", Access::Admin, getSettings);
    router.add("PUT", "/api/admin/settings", Access::Admin, putSettings);
    router.add("GET", "/api/admin/activity-logs", Access::Admin, activityLogs);
}

}  // namespace shoppro
