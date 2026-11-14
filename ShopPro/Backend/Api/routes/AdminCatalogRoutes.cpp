// Quan tri san pham va danh muc. Moi thay doi deu duoc kiem tra o server va ghi Activity Log.
#include <algorithm>

#include "Presenters.h"

namespace shoppro {
namespace {

std::int64_t idParam(const Request& req, const char* what) {
    const std::string& text = req.params.at("id");
    if (text.empty() || text.size() > 12 || text.find_first_not_of("0123456789") != std::string::npos) {
        throw ApiError(404, std::string("Không tìm thấy ") + what + ".");
    }
    return std::stoll(text);
}

std::int64_t boundedInteger(const Json& body, const char* key, std::int64_t low, std::int64_t high,
                            const char* label) {
    if (!body.contains(key)) throw ApiError(400, std::string("Thiếu trường '") + key + "'.");
    const std::int64_t value = jsonx::integer(body, key);
    if (value < low || value > high) {
        throw ApiError(400, std::string(label) + " phải nằm trong khoảng " + std::to_string(low) + " – " +
                                std::to_string(high) + ".");
    }
    return value;
}

// Anh san pham: duong dan noi bo (/img/...) hoac URL http(s). Chan javascript:, data: ...
std::string validImage(const std::string& image) {
    if (image.empty()) return image;
    const bool local = image.size() > 1 && image[0] == '/' && image[1] != '/' && image.find("..") == std::string::npos;
    const bool remote = image.rfind("https://", 0) == 0 || image.rfind("http://", 0) == 0;
    if (!local && !remote) throw ApiError(400, "Ảnh phải là đường dẫn nội bộ (/img/...) hoặc URL http/https.");
    return image;
}

// Doc & kiem tra toan bo truong san pham (dung cho tao moi va thay the PUT)
Json parseProduct(const Json& data, const Json& body) {
    const std::string name = jsonx::requiredString(body, "name", 120);
    if (name.size() < 2) throw ApiError(400, "Tên sản phẩm cần có ít nhất 2 ký tự.");
    const std::int64_t categoryId = jsonx::integer(body, "categoryId", -1);
    if (!view::findCategory(data, categoryId)) throw ApiError(400, "Danh mục không tồn tại.");
    return {{"name", name},
            {"categoryId", categoryId},
            {"price", boundedInteger(body, "price", 0, kMaxProductPrice, "Giá")},
            {"stock", boundedInteger(body, "stock", 0, kMaxStock, "Tồn kho")},
            {"description", jsonx::optionalString(body, "description", 2000)},
            {"image", validImage(jsonx::optionalString(body, "image", 500))},
            {"visible", jsonx::optionalBool(body, "visible", true)},
            {"featured", jsonx::optionalBool(body, "featured", false)},
            {"bestSelling", jsonx::optionalBool(body, "bestSelling", false)}};
}

HttpResponse listProducts(Ctx& ctx, Request&) {
    Json result = Json::array();
    for (const auto& product : ctx.data.at("products")) result.push_back(view::adminProduct(ctx.data, product));
    return {200, result, {}};
}

HttpResponse getProduct(Ctx& ctx, Request& req) {
    const Json* product = view::findProduct(ctx.data, idParam(req, "sản phẩm"));
    if (!product) throw ApiError(404, "Không tìm thấy sản phẩm.");
    return {200, view::adminProduct(ctx.data, *product), {}};
}

HttpResponse createProduct(Ctx& ctx, Request& req) {
    Json fields = parseProduct(ctx.data, req.body);
    Json next = ctx.data;
    std::int64_t id = 100;
    for (const auto& product : next.at("products")) id = std::max(id, jsonx::integer(product, "id"));
    ++id;
    if (id > 2147483647LL) throw ApiError(409, "Đã hết mã sản phẩm.");
    const std::string now = timeutil::nowIso();
    fields["id"] = id;
    fields["createdAt"] = now;
    fields["updatedAt"] = now;
    next["products"].push_back(fields);
    const Json response = view::adminProduct(next, fields);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.product.created", "product", std::to_string(id), "success",
              {{"name", fields["name"]}, {"price", fields["price"]}, {"stock", fields["stock"]}});
    return {201, response, {}};
}

HttpResponse updateProduct(Ctx& ctx, Request& req) {
    const std::int64_t id = idParam(req, "sản phẩm");
    Json fields = parseProduct(ctx.data, req.body);
    Json next = ctx.data;
    Json* product = view::findProduct(next, id);
    if (!product) throw ApiError(404, "Không tìm thấy sản phẩm.");
    Json changed = Json::array();
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        if (!product->contains(it.key()) || (*product)[it.key()] != it.value()) changed.push_back(it.key());
        (*product)[it.key()] = it.value();
    }
    (*product)["updatedAt"] = timeutil::nowIso();
    const Json response = view::adminProduct(next, *product);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.product.updated", "product", std::to_string(id), "success", {{"changed", changed}});
    return {200, response, {}};
}

HttpResponse updateStock(Ctx& ctx, Request& req) {
    const std::int64_t id = idParam(req, "sản phẩm");
    const std::int64_t stock = boundedInteger(req.body, "stock", 0, kMaxStock, "Tồn kho");
    Json next = ctx.data;
    Json* product = view::findProduct(next, id);
    if (!product) throw ApiError(404, "Không tìm thấy sản phẩm.");
    const std::int64_t before = jsonx::integer(*product, "stock");
    (*product)["stock"] = stock;
    (*product)["updatedAt"] = timeutil::nowIso();
    const Json response = view::adminProduct(next, *product);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.product.stock_updated", "product", std::to_string(id), "success",
              {{"before", before}, {"after", stock}});
    return {200, response, {}};
}

HttpResponse updateVisibility(Ctx& ctx, Request& req) {
    const std::int64_t id = idParam(req, "sản phẩm");
    if (!req.body.contains("visible") || !req.body["visible"].is_boolean()) throw ApiError(400, "Thiếu trường 'visible' (true/false).");
    const bool visible = req.body["visible"].get<bool>();
    Json next = ctx.data;
    Json* product = view::findProduct(next, id);
    if (!product) throw ApiError(404, "Không tìm thấy sản phẩm.");
    (*product)["visible"] = visible;
    (*product)["updatedAt"] = timeutil::nowIso();
    const Json response = view::adminProduct(next, *product);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.product.visibility_updated", "product", std::to_string(id), "success", {{"visible", visible}});
    return {200, response, {}};
}

HttpResponse deleteProduct(Ctx& ctx, Request& req) {
    const std::int64_t id = idParam(req, "sản phẩm");
    Json next = ctx.data;
    Json& products = next["products"];
    const auto it = std::find_if(products.begin(), products.end(), [&](const Json& p) { return jsonx::integer(p, "id") == id; });
    if (it == products.end()) throw ApiError(404, "Không tìm thấy sản phẩm.");
    const std::string name = it->value("name", "");
    products.erase(it);   // don hang cu giu ban chup ten/gia nen khong bi anh huong
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.product.deleted", "product", std::to_string(id), "success", {{"name", name}});
    return {200, {{"message", "Đã xóa sản phẩm."}}, {}};
}

// ------------------------------- Danh muc -------------------------------
Json parseCategory(const Json& data, const Json& body, std::int64_t selfId) {
    const std::string name = jsonx::requiredString(body, "name", 60);
    if (name.size() < 2) throw ApiError(400, "Tên danh mục cần có ít nhất 2 ký tự.");
    const std::string key = text::searchKey(name);
    for (const auto& category : data.at("categories")) {
        if (jsonx::integer(category, "id") != selfId && text::searchKey(category.value("name", "")) == key) {
            throw ApiError(409, "Tên danh mục đã tồn tại.");
        }
    }
    return {{"name", name}, {"description", jsonx::optionalString(body, "description", 200)}};
}

HttpResponse listCategories(Ctx& ctx, Request&) {
    Json result = Json::array();
    for (const auto& category : ctx.data.at("categories")) result.push_back(view::adminCategory(ctx.data, category));
    return {200, result, {}};
}

HttpResponse createCategory(Ctx& ctx, Request& req) {
    Json fields = parseCategory(ctx.data, req.body, -1);
    Json next = ctx.data;
    std::int64_t id = 0;
    for (const auto& category : next.at("categories")) id = std::max(id, jsonx::integer(category, "id"));
    fields["id"] = id + 1;
    next["categories"].push_back(fields);
    const Json response = view::adminCategory(next, fields);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.category.created", "category", std::to_string(id + 1), "success", {{"name", fields["name"]}});
    return {201, response, {}};
}

HttpResponse updateCategory(Ctx& ctx, Request& req) {
    const std::int64_t id = idParam(req, "danh mục");
    Json fields = parseCategory(ctx.data, req.body, id);
    Json next = ctx.data;
    Json* category = jsonx::findById(next["categories"], id);
    if (!category) throw ApiError(404, "Không tìm thấy danh mục.");
    (*category)["name"] = fields["name"];
    (*category)["description"] = fields["description"];
    const Json response = view::adminCategory(next, *category);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.category.updated", "category", std::to_string(id), "success", {{"name", fields["name"]}});
    return {200, response, {}};
}

HttpResponse deleteCategory(Ctx& ctx, Request& req) {
    const std::int64_t id = idParam(req, "danh mục");
    Json next = ctx.data;
    Json& categories = next["categories"];
    const auto it = std::find_if(categories.begin(), categories.end(), [&](const Json& c) { return jsonx::integer(c, "id") == id; });
    if (it == categories.end()) throw ApiError(404, "Không tìm thấy danh mục.");
    for (const auto& product : next.at("products")) {
        if (jsonx::integer(product, "categoryId") == id) {
            throw ApiError(409, "Danh mục còn sản phẩm. Hãy chuyển hoặc xóa sản phẩm trước.");
        }
    }
    const std::string name = it->value("name", "");
    categories.erase(it);
    ctx.commit(std::move(next));
    ctx.audit(req, "admin.category.deleted", "category", std::to_string(id), "success", {{"name", name}});
    return {200, {{"message", "Đã xóa danh mục."}}, {}};
}

}  // namespace

void registerAdminCatalogRoutes(Router& router) {
    router.add("GET", "/api/admin/products", Access::Admin, listProducts);
    router.add("POST", "/api/admin/products", Access::Admin, createProduct);
    router.add("GET", "/api/admin/products/:id", Access::Admin, getProduct);
    router.add("PUT", "/api/admin/products/:id", Access::Admin, updateProduct);
    router.add("PATCH", "/api/admin/products/:id/stock", Access::Admin, updateStock);
    router.add("PATCH", "/api/admin/products/:id/visibility", Access::Admin, updateVisibility);
    router.add("DELETE", "/api/admin/products/:id", Access::Admin, deleteProduct);

    router.add("GET", "/api/admin/categories", Access::Admin, listCategories);
    router.add("POST", "/api/admin/categories", Access::Admin, createCategory);
    router.add("PUT", "/api/admin/categories/:id", Access::Admin, updateCategory);
    router.add("DELETE", "/api/admin/categories/:id", Access::Admin, deleteCategory);
}

}  // namespace shoppro
