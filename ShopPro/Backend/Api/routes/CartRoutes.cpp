// Gio hang: dung CartController (danh sach lien ket doi + ProductManager + ProductSearch +
// CartProcessing) cua nhom lam loi xu ly. JSON chi la noi luu ben vung sau moi thay doi.
#include <algorithm>

#include "Presenters.h"

namespace shoppro {
namespace {

// Chay mot thao tac len gio hang: thanh cong thi luu xuong dia, that bai thi hoan tac RAM.
template <typename Operation>
HttpResponse mutateCart(Ctx& ctx, Request& req, const std::string& action, const std::string& targetId,
                        const Json& details, Operation operation) {
    ctx.carts.syncWithCatalog(req.userId, ctx.data);
    try {
        operation(ctx.carts.cart(req.userId, ctx.data));
        Json next = ctx.data;
        next["carts"][req.userId] = ctx.carts.lines(req.userId, ctx.data);
        ctx.commit(std::move(next));
    } catch (...) {
        ctx.carts.rebuild(req.userId, ctx.data);   // dua danh sach lien ket ve dung trang thai da luu
        throw;
    }
    ctx.audit(req, action, "cart", targetId, "success", details);
    return {200, view::cartPayload(ctx, req.userId), {}};
}

std::int64_t productIdParam(const Request& req) {
    const std::string& text = req.params.at("id");
    if (text.empty() || text.size() > 12 || text.find_first_not_of("0123456789") != std::string::npos) {
        throw ApiError(404, "Không tìm thấy sản phẩm.");
    }
    return std::stoll(text);
}

const Json& availableProduct(Ctx& ctx, std::int64_t productId) {
    const Json* product = view::findProduct(ctx.data, productId);
    if (!product || !product->value("visible", true)) {
        throw ApiError(404, "Sản phẩm không tồn tại hoặc đã ngừng bán.");
    }
    return *product;
}

HttpResponse getCart(Ctx& ctx, Request& req) { return {200, view::cartPayload(ctx, req.userId), {}}; }

HttpResponse addItem(Ctx& ctx, Request& req) {
    const std::int64_t productId = jsonx::integer(req.body, "productId", -1);
    const std::int64_t quantity = jsonx::integer(req.body, "quantity", 1);
    if (quantity < 1 || quantity > kMaxStock) throw ApiError(400, "Số lượng phải từ 1 đến " + std::to_string(kMaxStock) + ".");
    const Json& product = availableProduct(ctx, productId);
    const std::int64_t stock = jsonx::integer(product, "stock");
    const std::string name = product.value("name", "");
    const double price = static_cast<double>(jsonx::integer(product, "price"));
    if (stock < 1) throw ApiError(409, "Sản phẩm này đã hết hàng.");

    return mutateCart(ctx, req, "cart.item_added", std::to_string(productId),
                      {{"productId", productId}, {"quantity", quantity}},
                      [&](CartController& cart) {
        ctx.carts.syncWithCatalog(req.userId, ctx.data);
        const Node* existing = cart.timTheoId(static_cast<int>(productId));   // ProductSearch
        const std::int64_t already = existing ? existing->soLuong : 0;
        if (already + quantity > stock) {
            throw ApiError(409, "Kho chỉ còn " + std::to_string(stock) + " sản phẩm (trong giỏ đã có " +
                                    std::to_string(already) + ").");
        }
        const MaKetQua result = cart.themSanPham(static_cast<int>(productId), name, price, static_cast<int>(quantity));
        if (result != THANH_CONG && result != DA_CONG_DON) {
            throw ApiError(400, std::string("Không thể thêm vào giỏ: ") + moTaKetQua(result));
        }
    });
}

HttpResponse updateItem(Ctx& ctx, Request& req) {
    const std::int64_t productId = productIdParam(req);
    const std::int64_t quantity = jsonx::integer(req.body, "quantity", -1);
    if (quantity < 1 || quantity > kMaxStock) throw ApiError(400, "Số lượng phải từ 1 đến " + std::to_string(kMaxStock) + ". Dùng nút Xóa để bỏ sản phẩm.");
    const Json& product = availableProduct(ctx, productId);
    const std::int64_t stock = jsonx::integer(product, "stock");
    if (quantity > stock) throw ApiError(409, "Kho chỉ còn " + std::to_string(stock) + " sản phẩm.");

    return mutateCart(ctx, req, "cart.item_updated", std::to_string(productId),
                      {{"productId", productId}, {"quantity", quantity}},
                      [&](CartController& cart) {
        const MaKetQua result = cart.capNhatSoLuong(static_cast<int>(productId), static_cast<int>(quantity));
        if (result == ID_KHONG_TON_TAI) throw ApiError(404, "Sản phẩm này không có trong giỏ.");
        if (result != THANH_CONG) throw ApiError(400, std::string("Không thể cập nhật: ") + moTaKetQua(result));
    });
}

HttpResponse removeItem(Ctx& ctx, Request& req) {
    const std::int64_t productId = productIdParam(req);
    return mutateCart(ctx, req, "cart.item_removed", std::to_string(productId), {{"productId", productId}},
                      [&](CartController& cart) {
        if (productId > 2147483647LL || cart.xoaSanPham(static_cast<int>(productId)) != THANH_CONG) {
            throw ApiError(404, "Sản phẩm này không có trong giỏ.");
        }
    });
}

HttpResponse clearCart(Ctx& ctx, Request& req) {
    return mutateCart(ctx, req, "cart.cleared", req.userId, Json::object(),
                      [](CartController& cart) { cart.lamTrongGio(); });
}

// ---- Tinh nang DSA: sap xep / tim kiem / xem cau truc danh sach lien ket ----
HttpResponse sortCart(Ctx& ctx, Request& req) {
    const std::string by = jsonx::requiredString(req.body, "by", 20);
    return mutateCart(ctx, req, "cart.sorted", req.userId, {{"by", by}}, [&](CartController&) {
        if (!ctx.carts.sortBy(req.userId, ctx.data, by)) {
            throw ApiError(400, "Kiểu sắp xếp không hợp lệ. Dùng: price-asc, price-desc, name, id.");
        }
    });
}

HttpResponse searchCart(Ctx& ctx, Request& req) {
    const std::string query = req.query.count("q") ? text::trim(req.query.at("q")) : "";
    if (query.empty() || query.size() > 100) throw ApiError(400, "Từ khóa tìm kiếm cần từ 1 đến 100 ký tự.");
    ctx.carts.syncWithCatalog(req.userId, ctx.data);
    const std::vector<std::int64_t> ids = ctx.carts.searchByName(req.userId, ctx.data, query);   // ProductSearch
    Json payload = view::cartPayload(ctx, req.userId);
    Json matches = Json::array();
    for (const auto& item : payload["items"]) {
        const std::int64_t id = item["product"]["id"].get<std::int64_t>();
        if (std::find(ids.begin(), ids.end(), id) != ids.end()) matches.push_back(item);
    }
    return {200, {{"query", query}, {"items", matches}, {"total", matches.size()}}, {}};
}

HttpResponse structure(Ctx& ctx, Request& req) {
    const std::string direction = req.query.count("direction") ? req.query.at("direction") : "forward";
    if (direction != "forward" && direction != "backward") throw ApiError(400, "direction phải là forward hoặc backward.");
    ctx.carts.syncWithCatalog(req.userId, ctx.data);
    return {200, ctx.carts.structure(req.userId, ctx.data, direction == "forward"), {}};
}

}  // namespace

void registerCartRoutes(Router& router) {
    router.add("GET", "/api/cart", Access::Customer, getCart);
    router.add("DELETE", "/api/cart", Access::Customer, clearCart);
    router.add("POST", "/api/cart/items", Access::Customer, addItem);
    router.add("PATCH", "/api/cart/items/:id", Access::Customer, updateItem);
    router.add("DELETE", "/api/cart/items/:id", Access::Customer, removeItem);
    router.add("POST", "/api/cart/sort", Access::Customer, sortCart);
    router.add("GET", "/api/cart/search", Access::Customer, searchCart);
    router.add("GET", "/api/cart/structure", Access::Customer, structure);
}

}  // namespace shoppro
