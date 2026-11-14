// Route cong khai + tro ly mua sam: health, san pham, danh muc, AI chat.
#include <algorithm>
#include <regex>
#include <set>

#include "Presenters.h"

namespace shoppro {
namespace {

using view::publicProduct;

std::int64_t parseProductId(const std::string& text) {
    if (text.empty() || text.size() > 12 || text.find_first_not_of("0123456789") != std::string::npos) {
        throw ApiError(404, "Không tìm thấy sản phẩm.");
    }
    return std::stoll(text);
}

HttpResponse health(Ctx& ctx, Request&) {
    return {200,
            {{"status", "ok"},
             {"service", "ShopPro REST API"},
             {"products", ctx.data.at("products").size()},
             {"orders", ctx.data.at("orders").size()},
             {"time", timeutil::nowIso()}},
            {}};
}

HttpResponse listProducts(Ctx& ctx, Request& req) {
    const Json& data = ctx.data;
    const auto& query = req.query;
    const auto valueOf = [&](const char* key) { return query.count(key) ? query.at(key) : std::string(); };
    const std::string search = text::searchKey(valueOf("search"));
    const std::string category = valueOf("category");
    const std::int64_t minPrice = jsonx::queryInteger(query, "minPrice", 0);
    const std::int64_t maxPrice = jsonx::queryInteger(query, "maxPrice", kMaxProductPrice);
    const bool inStockOnly = valueOf("inStock") == "1" || valueOf("inStock") == "true";
    const bool featuredOnly = valueOf("featured") == "1" || valueOf("featured") == "true";
    const bool bestOnly = valueOf("bestSelling") == "1" || valueOf("bestSelling") == "true";

    std::vector<const Json*> picked;
    for (const auto& product : data.at("products")) {
        if (!product.value("visible", true)) continue;
        const std::int64_t price = jsonx::integer(product, "price");
        if (price < minPrice || price > maxPrice) continue;
        if (inStockOnly && jsonx::integer(product, "stock") < 1) continue;
        if (featuredOnly && !product.value("featured", false)) continue;
        if (bestOnly && !product.value("bestSelling", false)) continue;
        if (!category.empty()) {
            const Json* cat = view::findCategory(data, jsonx::integer(product, "categoryId"));
            const bool byName = cat && cat->value("name", "") == category;
            const bool byId = std::to_string(jsonx::integer(product, "categoryId")) == category;
            if (!byName && !byId) continue;
        }
        if (!search.empty()) {
            const std::string haystack = text::searchKey(product.value("name", "") + " " + product.value("description", ""));
            if (haystack.find(search) == std::string::npos) continue;
        }
        picked.push_back(&product);
    }

    const std::string sort = valueOf("sort");
    const auto price = [](const Json* p) { return jsonx::integer(*p, "price"); };
    if (sort == "price-asc") std::stable_sort(picked.begin(), picked.end(), [&](const Json* a, const Json* b) { return price(a) < price(b); });
    else if (sort == "price-desc") std::stable_sort(picked.begin(), picked.end(), [&](const Json* a, const Json* b) { return price(a) > price(b); });
    else if (sort == "newest") std::stable_sort(picked.begin(), picked.end(), [](const Json* a, const Json* b) { return a->value("createdAt", "") > b->value("createdAt", ""); });
    else if (sort == "name") std::stable_sort(picked.begin(), picked.end(), [](const Json* a, const Json* b) { return text::searchKey(a->value("name", "")) < text::searchKey(b->value("name", "")); });
    else if (sort == "popular") std::stable_sort(picked.begin(), picked.end(), [](const Json* a, const Json* b) { return a->value("bestSelling", false) > b->value("bestSelling", false); });

    // Phan trang: chan tran so khi page/limit rat lon
    const std::int64_t page = std::clamp<std::int64_t>(jsonx::queryInteger(query, "page", 1), 1, 1000000);
    const std::int64_t limit = std::clamp<std::int64_t>(jsonx::queryInteger(query, "limit", 12), 1, 100);
    Json items = Json::array();
    const std::size_t start = static_cast<std::size_t>((page - 1) * limit);
    for (std::size_t i = start; i < picked.size() && i < start + static_cast<std::size_t>(limit); ++i) {
        items.push_back(publicProduct(data, *picked[i]));
    }
    return {200, {{"items", items}, {"page", page}, {"limit", limit}, {"total", picked.size()}}, {}};
}

HttpResponse getProduct(Ctx& ctx, Request& req) {
    const Json* product = view::findProduct(ctx.data, parseProductId(req.params.at("id")));
    if (!product || !product->value("visible", true)) throw ApiError(404, "Không tìm thấy sản phẩm.");
    return {200, publicProduct(ctx.data, *product), {}};
}

HttpResponse relatedProducts(Ctx& ctx, Request& req) {
    const std::int64_t id = parseProductId(req.params.at("id"));
    const Json* selected = view::findProduct(ctx.data, id);
    if (!selected || !selected->value("visible", true)) throw ApiError(404, "Không tìm thấy sản phẩm.");
    Json related = Json::array();
    for (const auto& product : ctx.data.at("products")) {
        if (jsonx::integer(product, "id") != id && product.value("visible", true) &&
            jsonx::integer(product, "categoryId") == jsonx::integer(*selected, "categoryId")) {
            related.push_back(publicProduct(ctx.data, product));
            if (related.size() == 4) break;
        }
    }
    return {200, related, {}};
}

HttpResponse listCategories(Ctx& ctx, Request&) {
    Json result = Json::array();
    for (const auto& category : ctx.data.at("categories")) {
        int count = 0;
        for (const auto& product : ctx.data.at("products")) {
            if (product.value("visible", true) && jsonx::integer(product, "categoryId") == jsonx::integer(category, "id")) ++count;
        }
        result.push_back({{"id", jsonx::integer(category, "id")},
                          {"name", category.value("name", "")},
                          {"description", category.value("description", "")},
                          {"productCount", count}});
    }
    return {200, result, {}};
}

HttpResponse storeInfo(Ctx& ctx, Request&) {
    const Json& s = ctx.data.at("settings");
    return {200, {{"storeName", s.value("storeName", "ShopPro")},
                  {"supportEmail", s.value("supportEmail", "")},
                  {"supportPhone", s.value("supportPhone", "")},
                  {"currency", s.value("currency", "VND")}}, {}};
}

// ---- Tro ly mua sam (luat don gian, KHONG goi mo hinh ngoai) ----
// Doc ngan sach tu cau nhu "duoi 2 trieu", "500k", "1,5tr". Khong dung stod tren du lieu nguoi dung.
std::int64_t parseBudget(const std::string& folded) {
    static const std::regex pattern(R"(([0-9]{1,12})(?:[.,]([0-9]{1,3}))?\s*(trieu|tr|nghin|ngan|k)\b)");
    std::smatch match;
    if (!std::regex_search(folded, match, pattern)) return kMaxProductPrice;
    const std::int64_t whole = std::stoll(match[1].str());
    std::int64_t thousandths = 0;   // phan thap phan, don vi 1/1000
    if (match[2].matched) {
        std::string frac = match[2].str();
        while (frac.size() < 3) frac.push_back('0');
        thousandths = std::stoll(frac);
    }
    const std::string unit = match[3].str();
    const std::int64_t multiplier = (unit == "trieu" || unit == "tr") ? 1000000 : 1000;
    if (whole > kMaxProductPrice / multiplier) return kMaxProductPrice;
    return whole * multiplier + thousandths * multiplier / 1000;
}

HttpResponse aiChat(Ctx& ctx, Request& req) {
    const std::string raw = jsonx::requiredString(req.body, "message", 1000);
    const std::string message = text::searchKey(raw);
    const bool comparison = message.find("so sanh") != std::string::npos;
    const bool cheapest = message.find("re nhat") != std::string::npos || message.find("gia re") != std::string::npos;
    const bool bestSeller = message.find("ban chay") != std::string::npos;
    const std::int64_t maxPrice = parseBudget(message);

    std::string categoryName;
    std::int64_t categoryId = -1;
    for (const auto& category : ctx.data.at("categories")) {
        const std::string key = text::searchKey(category.value("name", ""));
        if (!key.empty() && message.find(key) != std::string::npos) {
            categoryName = category.value("name", "");
            categoryId = jsonx::integer(category, "id");
        }
    }

    // Bo tu "dem" (khong mang y nghia san pham) de khong khop nham vao mo ta
    static const std::set<std::string> stopWords{
        "goi", "duoi", "tren", "trieu", "nghin", "ngan", "cho", "toi", "can", "muon", "mua", "tim", "san", "pham",
        "nao", "tot", "nhat", "hang", "mot", "cai", "nhung", "khoang", "tam", "gia", "con", "voi", "cua", "cac",
        "ban", "chay", "re", "so", "sanh", "va", "hay", "giup", "minh", "shop", "xin", "oi", "nhe", "the"};
    std::vector<std::string> words;
    for (const auto& word : text::split(message, ' ')) {
        if (word.size() >= 3 && word.find_first_of("0123456789") == std::string::npos && !stopWords.count(word)) {
            words.push_back(word);
        }
    }

    struct Scored { const Json* product; int score; int wordScore; };
    std::vector<Scored> scored;
    for (const auto& product : ctx.data.at("products")) {
        if (!product.value("visible", true) || jsonx::integer(product, "stock") < 1) continue;
        if (jsonx::integer(product, "price") > maxPrice) continue;
        if (categoryId >= 0 && jsonx::integer(product, "categoryId") != categoryId) continue;
        const std::string haystack = text::searchKey(product.value("name", "") + " " + product.value("description", ""));
        int wordScore = 0;
        for (const auto& word : words) {
            if (haystack.find(word) != std::string::npos) wordScore += 2;
        }
        int score = wordScore;
        if (bestSeller && product.value("bestSelling", false)) score += 2;
        if (product.value("featured", false)) score += 1;
        scored.push_back({&product, score, wordScore});
    }
    // Co tu khoa trung khop -> chi giu cac san pham co khop; khong thi goi y theo do noi bat
    const bool anyKeyword = std::any_of(scored.begin(), scored.end(), [](const Scored& s) { return s.wordScore > 0; });
    if (anyKeyword) {
        scored.erase(std::remove_if(scored.begin(), scored.end(), [](const Scored& s) { return s.wordScore == 0; }), scored.end());
    }
    std::stable_sort(scored.begin(), scored.end(), [&](const Scored& a, const Scored& b) {
        if (cheapest) return jsonx::integer(*a.product, "price") < jsonx::integer(*b.product, "price");
        return a.score > b.score;
    });
    const std::size_t take = comparison ? 2 : 3;
    Json products = Json::array();
    for (std::size_t i = 0; i < scored.size() && i < take; ++i) products.push_back(publicProduct(ctx.data, *scored[i].product));

    std::string answer;
    if (products.empty()) {
        answer = "Hiện chưa có sản phẩm còn hàng phù hợp với tiêu chí của bạn. Bạn thử tăng ngân sách hoặc đổi danh mục nhé.";
    } else if (comparison) {
        answer = "Mình chọn các sản phẩm còn hàng dưới đây để bạn đặt cạnh nhau và so sánh giá, tồn kho.";
    } else {
        answer = "Dựa trên danh mục, ngân sách và tồn kho hiện tại, đây là những sản phẩm phù hợp nhất.";
    }
    Json filters{{"maxPrice", maxPrice >= kMaxProductPrice ? Json(nullptr) : Json(maxPrice)},
                 {"category", categoryName.empty() ? Json(nullptr) : Json(categoryName)}};
    return {200, {{"answer", answer}, {"products", products}, {"comparison", comparison}, {"filters", filters}}, {}};
}

}  // namespace

void registerPublicRoutes(Router& router) {
    router.add("GET", "/api/health", Access::Public, health);
    router.add("GET", "/api/store", Access::Public, storeInfo);
    router.add("GET", "/api/products", Access::Public, listProducts);
    router.add("GET", "/api/products/:id", Access::Public, getProduct);
    router.add("GET", "/api/products/:id/related", Access::Public, relatedProducts);
    router.add("GET", "/api/categories", Access::Public, listCategories);
    router.add("POST", "/api/ai/chat", Access::Customer, aiChat);
}

}  // namespace shoppro
