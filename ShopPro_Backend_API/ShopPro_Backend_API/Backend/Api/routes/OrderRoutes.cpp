// Don hang cua khach hang: dat hang (tru kho), xem, huy don dang cho.
#include <algorithm>

#include "Presenters.h"

namespace shoppro {
namespace {

HttpResponse listMyOrders(Ctx& ctx, Request& req) {
    std::vector<const Json*> mine;
    for (const auto& order : ctx.data.at("orders")) {
        if (order.value("customerId", "") == req.userId) mine.push_back(&order);
    }
    std::stable_sort(mine.begin(), mine.end(), [](const Json* a, const Json* b) {
        return a->value("createdAt", "") > b->value("createdAt", "");
    });
    Json result = Json::array();
    for (const Json* order : mine) result.push_back(view::customerOrder(ctx.data, *order));
    return {200, result, {}};
}

HttpResponse getMyOrder(Ctx& ctx, Request& req) {
    const Json* order = jsonx::findByStringId(ctx.data.at("orders"), req.params.at("id"));
    if (!order || order->value("customerId", "") != req.userId) throw ApiError(404, "Không tìm thấy đơn hàng.");
    return {200, view::customerOrder(ctx.data, *order), {}};
}

HttpResponse placeOrder(Ctx& ctx, Request& req) {
    const std::string customerName = req.body.contains("customerName")
                                         ? jsonx::requiredString(req.body, "customerName", 80)
                                         : req.userName;
    const std::string phone = jsonx::requiredString(req.body, "phone", 40);
    const std::string address = jsonx::requiredString(req.body, "shippingAddress", 300);
    const std::string note = jsonx::optionalString(req.body, "note", 500);
    std::string payment = jsonx::optionalString(req.body, "paymentMethod", 20);
    if (payment.empty()) payment = "cod";
    if (customerName.size() < 2) throw ApiError(400, "Họ tên người nhận cần có ít nhất 2 ký tự.");
    if (!text::validPhone(phone)) throw ApiError(400, "Số điện thoại không hợp lệ.");
    if (address.size() < 8) throw ApiError(400, "Địa chỉ giao hàng cần ghi rõ hơn (ít nhất 8 ký tự).");
    if (payment != "cod" && payment != "bank") throw ApiError(400, "Phương thức thanh toán không hợp lệ.");

    ctx.carts.syncWithCatalog(req.userId, ctx.data);
    const Json lines = ctx.carts.lines(req.userId, ctx.data);   // copy: an toan sau commit
    if (lines.empty()) throw ApiError(409, "Giỏ hàng đang trống.");

    // (Tuy chon) client gui danh sach de doi chieu: phai khop gio hang tren server
    if (req.body.contains("items")) {
        if (!req.body["items"].is_array() || req.body["items"].size() != lines.size()) {
            throw ApiError(409, "Giỏ hàng đã thay đổi. Hãy tải lại giỏ trước khi đặt hàng.");
        }
        for (const auto& requested : req.body["items"]) {
            bool same = false;
            for (const auto& line : lines) {
                if (jsonx::integer(line, "productId") == jsonx::integer(requested, "productId", -1) &&
                    jsonx::integer(line, "quantity") == jsonx::integer(requested, "quantity", -1)) {
                    same = true;
                }
            }
            if (!same) throw ApiError(409, "Giỏ hàng đã thay đổi. Hãy tải lại giỏ trước khi đặt hàng.");
        }
    }

    Json next = ctx.data;
    Json orderLines = Json::array();
    std::int64_t total = 0;
    for (const auto& line : lines) {
        Json* product = view::findProduct(next, jsonx::integer(line, "productId"));
        if (!product || !product->value("visible", true)) throw ApiError(409, "Có sản phẩm trong giỏ không còn được bán.");
        const std::int64_t quantity = jsonx::integer(line, "quantity");
        const std::int64_t stock = jsonx::integer(*product, "stock");
        if (quantity < 1 || quantity > stock) {
            throw ApiError(409, "Sản phẩm \"" + product->value("name", "") + "\" chỉ còn " + std::to_string(stock) +
                                    " trong kho. Vui lòng giảm số lượng.");
        }
        const std::int64_t unitPrice = jsonx::integer(*product, "price");
        total = view::checkedAdd(total, view::checkedMultiply(unitPrice, quantity));
        orderLines.push_back({{"productId", jsonx::integer(*product, "id")},
                              {"productName", product->value("name", "")},
                              {"image", product->value("image", "")},
                              {"unitPrice", unitPrice},
                              {"quantity", quantity}});
        (*product)["stock"] = stock - quantity;
        (*product)["updatedAt"] = timeutil::nowIso();
    }

    const std::string now = timeutil::nowIso();
    Json order{{"id", view::newOrderId(next.at("orders"))},
               {"customerId", req.userId},
               {"customerName", customerName},
               {"email", req.userEmail},
               {"phone", phone},
               {"shippingAddress", address},
               {"note", note},
               {"paymentMethod", payment},
               {"createdAt", now},
               {"updatedAt", now},
               {"status", "pending"},
               {"items", orderLines},
               {"stockReserved", true},
               {"total", total}};
    next["orders"].push_back(order);
    next["carts"][req.userId] = Json::array();
    if (Json* user = jsonx::findByStringId(next["users"], req.userId)) {   // chi dien cac truong con trong
        if (user->value("phone", "").empty()) (*user)["phone"] = phone;
        if (user->value("address", "").empty()) (*user)["address"] = address;
    }
    const std::string orderId = order.value("id", "");
    const Json response = view::customerOrder(next, order);
    ctx.commit(std::move(next));                      // loi ghi dia -> throw, gio hang trong RAM van nguyen
    ctx.carts.cart(req.userId, ctx.data).lamTrongGio();
    ctx.audit(req, "order.placed", "order", orderId, "success",
              {{"total", total}, {"itemCount", orderLines.size()}, {"paymentMethod", payment}});
    return {201, response, {}};
}

HttpResponse cancelMyOrder(Ctx& ctx, Request& req) {
    const std::string orderId = req.params.at("id");
    Json next = ctx.data;
    Json* order = jsonx::findByStringId(next["orders"], orderId);
    if (!order || order->value("customerId", "") != req.userId) throw ApiError(404, "Không tìm thấy đơn hàng.");
    if (order->value("status", "") != "pending") {
        throw ApiError(409, "Chỉ có thể hủy đơn đang chờ xác nhận. Vui lòng liên hệ cửa hàng.");
    }
    view::restoreStockOnCancel(next, *order);
    (*order)["status"] = "cancelled";
    (*order)["updatedAt"] = timeutil::nowIso();
    const Json response = view::customerOrder(next, *order);
    ctx.commit(std::move(next));
    ctx.audit(req, "order.cancelled", "order", orderId, "success");
    return {200, response, {}};
}

}  // namespace

void registerOrderRoutes(Router& router) {
    router.add("GET", "/api/orders", Access::Customer, listMyOrders);
    router.add("POST", "/api/orders", Access::Customer, placeOrder);
    router.add("GET", "/api/orders/:id", Access::Customer, getMyOrder);
    router.add("POST", "/api/orders/:id/cancel", Access::Customer, cancelMyOrder);
}

}  // namespace shoppro
