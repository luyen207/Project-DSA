#include "Presenters.h"

#include <cctype>

namespace shoppro::view {

const Json* findCategory(const Json& data, std::int64_t id) { return jsonx::findById(data.at("categories"), id); }
const Json* findProduct(const Json& data, std::int64_t id) { return jsonx::findById(data.at("products"), id); }
Json* findProduct(Json& data, std::int64_t id) { return jsonx::findById(data["products"], id); }

Json publicProduct(const Json& data, const Json& product) {
    const Json* category = findCategory(data, jsonx::integer(product, "categoryId"));
    const std::int64_t stock = jsonx::integer(product, "stock");
    Json result{{"id", jsonx::integer(product, "id")},
                {"name", product.value("name", "")},
                {"price", jsonx::integer(product, "price")},
                {"stock", stock},
                {"description", product.value("description", "")},
                {"image", product.value("image", "")},
                {"categoryId", jsonx::integer(product, "categoryId")},
                {"category", category ? category->value("name", "Khác") : "Khác"},
                {"createdAt", product.value("createdAt", "")},
                {"featured", product.value("featured", false)},
                {"bestSelling", product.value("bestSelling", false)}};
    if (stock == 0) result["badge"] = "Tạm hết hàng";
    else if (product.value("bestSelling", false)) result["badge"] = "Bán chạy";
    else if (product.value("featured", false)) result["badge"] = "Nổi bật";
    return result;
}

Json adminProduct(const Json& data, const Json& product) {
    Json result = product;
    const Json* category = findCategory(data, jsonx::integer(product, "categoryId"));
    result["categoryName"] = category ? category->value("name", "Khác") : "Khác";
    result["updatedAt"] = product.value("updatedAt", product.value("createdAt", ""));
    return result;
}

Json adminCategory(const Json& data, const Json& category) {
    int count = 0;
    for (const auto& product : data.at("products")) {
        if (jsonx::integer(product, "categoryId") == jsonx::integer(category, "id")) ++count;
    }
    return {{"id", jsonx::integer(category, "id")},
            {"name", category.value("name", "")},
            {"description", category.value("description", "")},
            {"productCount", count}};
}

static Json orderLine(const Json& item) {
    return {{"productId", jsonx::integer(item, "productId")},
            {"productName", item.value("productName", "")},
            {"image", item.value("image", "")},
            {"unitPrice", jsonx::integer(item, "unitPrice")},
            {"quantity", jsonx::integer(item, "quantity")}};
}

Json adminOrder(const Json& order) {
    Json items = Json::array();
    for (const auto& item : order.value("items", Json::array())) items.push_back(orderLine(item));
    return {{"id", order.value("id", "")},
            {"customerId", order.value("customerId", "")},
            {"customerName", order.value("customerName", "")},
            {"email", order.value("email", "")},
            {"phone", order.value("phone", "")},
            {"address", order.value("shippingAddress", "")},
            {"note", order.value("note", "")},
            {"paymentMethod", order.value("paymentMethod", "cod")},
            {"createdAt", order.value("createdAt", "")},
            {"updatedAt", order.value("updatedAt", order.value("createdAt", ""))},
            {"status", order.value("status", "pending")},
            {"items", items},
            {"total", jsonx::integer(order, "total")}};
}

Json customerOrder(const Json& data, const Json& order) {
    Json items = Json::array();
    for (const auto& item : order.value("items", Json::array())) {
        const Json* current = findProduct(data, jsonx::integer(item, "productId"));
        Json snapshot = current ? publicProduct(data, *current) : Json::object();
        snapshot["id"] = jsonx::integer(item, "productId");
        snapshot["name"] = item.value("productName", snapshot.value("name", "Sản phẩm"));
        snapshot["image"] = item.value("image", snapshot.value("image", ""));
        snapshot["price"] = jsonx::integer(item, "unitPrice");
        items.push_back({{"product", snapshot}, {"quantity", jsonx::integer(item, "quantity")}});
    }
    return {{"id", order.value("id", "")},
            {"createdAt", order.value("createdAt", "")},
            {"updatedAt", order.value("updatedAt", order.value("createdAt", ""))},
            {"status", order.value("status", "pending")},
            {"items", items},
            {"total", jsonx::integer(order, "total")},
            {"shippingAddress", order.value("shippingAddress", "")},
            {"phone", order.value("phone", "")},
            {"note", order.value("note", "")},
            {"paymentMethod", order.value("paymentMethod", "cod")}};
}

Json publicUser(const Json& user) {
    return {{"id", user.value("id", "")},
            {"name", user.value("name", "")},
            {"email", user.value("email", "")},
            {"phone", user.value("phone", "")},
            {"address", user.value("address", "")},
            {"role", "customer"}};
}

Json adminUser(const Json& user) {
    return {{"id", user.value("id", "")},
            {"name", user.value("name", "")},
            {"email", user.value("email", "")},
            {"role", "admin"}};
}

Json adminCustomer(const Json& data, const Json& user) {
    std::int64_t orders = 0;
    std::int64_t spent = 0;
    std::string lastOrder;
    const std::string id = user.value("id", "");
    for (const auto& order : data.at("orders")) {
        if (order.value("customerId", "") != id) continue;
        ++orders;
        if (order.value("status", "") == "completed") spent += jsonx::integer(order, "total");
        const std::string created = order.value("createdAt", "");
        if (created > lastOrder) lastOrder = created;
    }
    return {{"id", id},
            {"name", user.value("name", "")},
            {"email", user.value("email", "")},
            {"phone", user.value("phone", "")},
            {"address", user.value("address", "")},
            {"joinedAt", user.value("joinedAt", "")},
            {"status", user.value("status", "active")},
            {"orderCount", orders},
            {"totalSpent", spent},
            {"lastOrderAt", lastOrder}};
}

std::int64_t checkedMultiply(std::int64_t price, std::int64_t quantity) {
    if (price < 0 || quantity < 0) throw ApiError(400, "Giá hoặc số lượng không hợp lệ.");
    if (quantity != 0 && price > kMaxSafeTotal / quantity) throw ApiError(400, "Giá trị vượt quá giới hạn hỗ trợ.");
    return price * quantity;
}

std::int64_t checkedAdd(std::int64_t total, std::int64_t amount) {
    if (amount > kMaxSafeTotal - total) throw ApiError(400, "Tổng giá trị vượt quá giới hạn hỗ trợ.");
    return total + amount;
}

std::string newOrderId(const Json& orders) {
    const std::string date = timeutil::todayIso();   // YYYY-MM-DD
    const std::string stamp = date.substr(2, 2) + date.substr(5, 2) + date.substr(8, 2);
    for (int attempt = 0; attempt < 2000; ++attempt) {
        std::string suffix = crypto::randomHex(attempt < 1000 ? 2 : 3);
        for (auto& c : suffix) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        const std::string id = "SP-" + stamp + "-" + suffix;
        if (!jsonx::findByStringId(orders, id)) return id;
    }
    throw ApiError(500, "Không thể tạo mã đơn hàng.");
}

void restoreStockOnCancel(Json& data, Json& order) {
    if (!order.value("stockReserved", false)) return;
    // Kiem tra truoc, ghi sau: khong de tra mot nua ton kho roi moi bao loi
    for (const auto& item : order.value("items", Json::array())) {
        const Json* product = findProduct(data, jsonx::integer(item, "productId"));
        if (!product) continue;
        if (jsonx::integer(*product, "stock") + jsonx::integer(item, "quantity") > kMaxStock) {
            throw ApiError(409, "Không thể hủy vì trả hàng về kho sẽ vượt giới hạn tồn kho.");
        }
    }
    for (const auto& item : order.value("items", Json::array())) {
        Json* product = findProduct(data, jsonx::integer(item, "productId"));
        if (!product) continue;
        (*product)["stock"] = jsonx::integer(*product, "stock") + jsonx::integer(item, "quantity");
    }
    order["stockReserved"] = false;
}

bool isTerminalStatus(const std::string& status) { return status == "completed" || status == "cancelled"; }

bool allowedTransition(const std::string& current, const std::string& next) {
    if (current == "pending") return next == "confirmed" || next == "cancelled";
    if (current == "confirmed") return next == "shipping" || next == "cancelled";
    if (current == "shipping") return next == "completed";
    return false;
}

Json cartPayload(Ctx& ctx, const std::string& userId) {
    ctx.carts.syncWithCatalog(userId, ctx.data);
    CartController& controller = ctx.carts.cart(userId, ctx.data);
    Json items = Json::array();
    std::int64_t total = 0;
    std::int64_t quantity = 0;
    const Json lines = ctx.carts.lines(userId, ctx.data);   // theo thu tu head -> tail cua danh sach lien ket
    for (const auto& line : lines) {
        const Json* product = findProduct(ctx.data, jsonx::integer(line, "productId"));
        if (!product) continue;
        const std::int64_t qty = jsonx::integer(line, "quantity");
        const std::int64_t lineTotal = checkedMultiply(jsonx::integer(*product, "price"), qty);
        total = checkedAdd(total, lineTotal);
        quantity += qty;
        items.push_back({{"product", publicProduct(ctx.data, *product)},
                         {"quantity", qty},
                         {"lineTotal", lineTotal},
                         {"exceedsStock", qty > jsonx::integer(*product, "stock")}});
    }
    return {{"userId", userId},
            {"items", items},
            {"total", total},
            {"totalQuantity", quantity},
            {"distinctCount", controller.soLoaiSanPham()},
            {"dsa", {{"size", controller.danhSach().soNode},
                     {"head", controller.danhSach().head ? Json(controller.danhSach().head->id) : Json(nullptr)},
                     {"tail", controller.danhSach().tail ? Json(controller.danhSach().tail->id) : Json(nullptr)},
                     {"linksValid", controller.lienKetHopLe()}}}};
}

}  // namespace shoppro::view
