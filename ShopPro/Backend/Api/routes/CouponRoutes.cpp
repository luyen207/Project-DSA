// Ma giam gia: khach xem ma dung duoc va kiem tra ma tren gio hang hien tai;
// Admin tao / sua / bat tat / xoa ma. Moi thay doi cua Admin deu ghi Activity Log.
#include <algorithm>
#include <vector>

#include "../Coupons.h"
#include "Presenters.h"

namespace shoppro {
namespace {

// ---------------- Khach hang ----------------

HttpResponse listAvailable(Ctx& ctx, Request& req) {
    std::vector<Json> views;
    if (ctx.data.contains("coupons") && ctx.data["coupons"].is_array()) {
        for (const auto& item : ctx.data["coupons"]) {
            if (coupon::visibleToCustomer(ctx.data, item, req.userId)) views.push_back(coupon::customerView(ctx.data, item));
        }
    }
    std::stable_sort(views.begin(), views.end(), [](const Json& a, const Json& b) {
        return jsonx::integer(a, "percent") > jsonx::integer(b, "percent");
    });
    return {200, Json(views), {}};
}

// Kiem tra ma tren gio hang HIEN TAI tren server (khong tin so tien do trinh duyet gui).
// Khong thay doi gio hang hay du lieu: ma chi thuc su bi "dung" luc dat hang.
HttpResponse validateCoupon(Ctx& ctx, Request& req) {
    const std::string code = jsonx::optionalString(req.body, "code", 40);
    if (code.empty()) throw ApiError(400, "Vui lòng nhập mã giảm giá.");
    const Json cart = view::cartPayload(ctx, req.userId);
    if (cart.at("items").empty()) throw ApiError(409, "Giỏ hàng đang trống, hãy thêm sản phẩm trước khi dùng mã.");
    const coupon::Applied applied = coupon::apply(ctx.data, code, req.userId, jsonx::integer(cart, "total"));
    return {200,
            {{"code", applied.code},
             {"description", applied.description},
             {"percent", applied.percent},
             {"minOrder", applied.minOrder},
             {"subtotal", applied.subtotal},
             {"discount", applied.discount},
             {"total", applied.total}},
            {}};
}

// ---------------- Admin ----------------

std::string codeParam(const Request& req) {
    const std::string code = coupon::cleanCode(req.params.at("code"));
    if (!coupon::validCodeFormat(code)) throw ApiError(404, "Không tìm thấy mã giảm giá.");
    return code;
}

HttpResponse adminList(Ctx& ctx, Request&) {
    std::vector<Json> views;
    if (ctx.data.contains("coupons") && ctx.data["coupons"].is_array()) {
        for (const auto& item : ctx.data["coupons"]) views.push_back(coupon::adminView(ctx.data, item));
    }
    std::stable_sort(views.begin(), views.end(), [](const Json& a, const Json& b) {
        return a.value("createdAt", "") > b.value("createdAt", "");
    });
    return {200, Json(views), {}};
}

HttpResponse adminCreate(Ctx& ctx, Request& req) {
    const std::string code = coupon::cleanCode(jsonx::requiredString(req.body, "code", 40));
    if (!coupon::validCodeFormat(code)) {
        throw ApiError(400, "Mã chỉ gồm chữ không dấu, số, gạch ngang hoặc gạch dưới (3–20 ký tự).");
    }
    Json fields = coupon::parseFields(req.body);
    if (coupon::find(ctx.data, code)) throw ApiError(409, "Mã giảm giá này đã tồn tại.");

    const std::string now = timeutil::nowIso();
    fields["code"] = code;
    fields["createdAt"] = now;
    fields["updatedAt"] = now;
    Json next = ctx.data;
    if (!next.contains("coupons") || !next["coupons"].is_array()) next["coupons"] = Json::array();
    next["coupons"].push_back(fields);
    const Json response = coupon::adminView(next, fields);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.coupon.created", "coupon", code, "success",
              {{"percent", fields["percent"]}, {"minOrder", fields["minOrder"]}, {"usageLimit", fields["usageLimit"]}});
    return {201, response, {}};
}

HttpResponse adminUpdate(Ctx& ctx, Request& req) {
    const std::string code = codeParam(req);
    Json fields = coupon::parseFields(req.body);
    Json next = ctx.data;
    Json* existing = coupon::find(next, code);
    if (!existing) throw ApiError(404, "Không tìm thấy mã giảm giá.");
    for (const char* key : {"description", "percent", "minOrder", "usageLimit", "oncePerCustomer", "active"}) {
        (*existing)[key] = fields[key];
    }
    (*existing)["updatedAt"] = timeutil::nowIso();
    const Json response = coupon::adminView(next, *existing);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.coupon.updated", "coupon", code, "success",
              {{"percent", fields["percent"]}, {"minOrder", fields["minOrder"]}, {"usageLimit", fields["usageLimit"]},
               {"active", fields["active"]}});
    return {200, response, {}};
}

HttpResponse adminSetStatus(Ctx& ctx, Request& req) {
    const std::string code = codeParam(req);
    if (!req.body.contains("active") || !req.body["active"].is_boolean()) {
        throw ApiError(400, "Trường 'active' phải là true hoặc false.");
    }
    const bool active = req.body["active"].get<bool>();
    Json next = ctx.data;
    Json* existing = coupon::find(next, code);
    if (!existing) throw ApiError(404, "Không tìm thấy mã giảm giá.");
    (*existing)["active"] = active;
    (*existing)["updatedAt"] = timeutil::nowIso();
    const Json response = coupon::adminView(next, *existing);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.coupon.status_changed", "coupon", code, "success", {{"active", active}});
    return {200, response, {}};
}

HttpResponse adminDelete(Ctx& ctx, Request& req) {
    const std::string code = codeParam(req);
    if (!coupon::find(ctx.data, code)) throw ApiError(404, "Không tìm thấy mã giảm giá.");
    // Don da dung ma (ke ca don da huy) giu ban ghi ma de doi chieu -> chi cho xoa ma chua tung duoc dung
    std::int64_t everUsed = 0;
    for (const auto& order : ctx.data.at("orders")) if (order.value("couponCode", "") == code) ++everUsed;
    if (everUsed > 0) {
        throw ApiError(409, "Mã đã được dùng trong " + std::to_string(everUsed) + " đơn hàng nên không thể xóa. Hãy tắt mã thay vì xóa.");
    }
    Json next = ctx.data;
    Json& list = next["coupons"];
    const auto it = std::find_if(list.begin(), list.end(), [&](const Json& item) { return item.value("code", "") == code; });
    if (it != list.end()) list.erase(it);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.coupon.deleted", "coupon", code, "success");
    return {200, {{"deleted", code}}, {}};
}

}  // namespace

void registerCouponRoutes(Router& router) {
    router.add("GET", "/api/coupons", Access::Customer, listAvailable);
    router.add("POST", "/api/coupons/validate", Access::Customer, validateCoupon);

    router.add("GET", "/api/admin/coupons", Access::Admin, adminList);
    router.add("POST", "/api/admin/coupons", Access::Admin, adminCreate);
    router.add("PUT", "/api/admin/coupons/:code", Access::Admin, adminUpdate);
    router.add("PATCH", "/api/admin/coupons/:code/status", Access::Admin, adminSetStatus);
    router.add("DELETE", "/api/admin/coupons/:code", Access::Admin, adminDelete);
}

}  // namespace shoppro
