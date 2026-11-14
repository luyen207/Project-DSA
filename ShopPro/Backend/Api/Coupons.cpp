#include "Coupons.h"

#include <algorithm>
#include <cctype>

namespace shoppro::coupon {

std::string cleanCode(const std::string& raw) {
    std::string result = text::trim(raw);
    for (auto& c : result) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return result;
}

bool validCodeFormat(const std::string& code) {
    if (code.size() < 3 || code.size() > 20) return false;
    for (const unsigned char c : code) {
        if (!std::isalnum(c) && c != '-' && c != '_') return false;
        if (c > 127) return false;
    }
    return true;
}

std::int64_t percentOf(std::int64_t amount, std::int64_t percent) {
    if (amount <= 0 || percent <= 0) return 0;
    // amount/100*percent + (amount%100)*percent/100 : khong bao gio tran so voi amount <= 2^53
    return (amount / 100) * percent + ((amount % 100) * percent) / 100;
}

std::string formatMoney(std::int64_t amount) {
    const bool negative = amount < 0;
    std::string digits = std::to_string(negative ? -amount : amount);
    std::string grouped;
    for (std::size_t i = 0; i < digits.size(); ++i) {
        if (i > 0 && (digits.size() - i) % 3 == 0) grouped += '.';
        grouped += digits[i];
    }
    return (negative ? "-" : "") + grouped + "đ";
}

const Json* find(const Json& data, const std::string& code) {
    if (!data.contains("coupons") || !data["coupons"].is_array()) return nullptr;
    for (const auto& item : data["coupons"]) {
        if (item.value("code", "") == code) return &item;
    }
    return nullptr;
}

Json* find(Json& data, const std::string& code) {
    if (!data.contains("coupons") || !data["coupons"].is_array()) return nullptr;
    for (auto& item : data["coupons"]) {
        if (item.value("code", "") == code) return &item;
    }
    return nullptr;
}

namespace {
bool counts(const Json& order, const std::string& code) {
    return order.value("couponCode", "") == code && order.value("status", "") != "cancelled";
}
}  // namespace

std::int64_t usedCount(const Json& data, const std::string& code) {
    std::int64_t count = 0;
    for (const auto& order : data.at("orders")) if (counts(order, code)) ++count;
    return count;
}

std::int64_t usedBy(const Json& data, const std::string& code, const std::string& userId) {
    std::int64_t count = 0;
    for (const auto& order : data.at("orders")) {
        if (counts(order, code) && order.value("customerId", "") == userId) ++count;
    }
    return count;
}

std::int64_t discountGiven(const Json& data, const std::string& code) {
    std::int64_t sum = 0;
    for (const auto& order : data.at("orders")) {
        if (counts(order, code)) sum += jsonx::integer(order, "discount");
    }
    return sum;
}

Applied apply(const Json& data, const std::string& rawCode, const std::string& userId, std::int64_t subtotal) {
    const std::string code = cleanCode(rawCode);
    if (code.empty()) throw ApiError(400, "Vui lòng nhập mã giảm giá.");
    const Json* found = validCodeFormat(code) ? find(data, code) : nullptr;
    if (!found) throw ApiError(404, "Mã giảm giá không tồn tại.");
    if (!found->value("active", true)) throw ApiError(409, "Mã giảm giá này đang tạm ngưng.");

    const std::int64_t limit = jsonx::integer(*found, "usageLimit");
    if (limit > 0 && usedCount(data, code) >= limit) throw ApiError(409, "Mã giảm giá đã hết lượt sử dụng.");
    if (found->value("oncePerCustomer", true) && usedBy(data, code, userId) > 0) {
        throw ApiError(409, "Bạn đã dùng mã này rồi (mỗi khách chỉ được dùng 1 lần).");
    }
    const std::int64_t minOrder = jsonx::integer(*found, "minOrder");
    if (subtotal < minOrder) {
        throw ApiError(409, "Mã " + code + " áp dụng cho đơn từ " + formatMoney(minOrder) + ". Bạn cần mua thêm " +
                                formatMoney(minOrder - subtotal) + ".");
    }
    const std::int64_t percent = jsonx::integer(*found, "percent");
    const std::int64_t discount = percentOf(subtotal, percent);
    if (discount <= 0) throw ApiError(409, "Giá trị đơn hàng quá nhỏ để áp dụng mã này.");

    Applied applied;
    applied.code = code;
    applied.description = found->value("description", "");
    applied.percent = percent;
    applied.minOrder = minOrder;
    applied.subtotal = subtotal;
    applied.discount = discount;
    applied.total = subtotal - discount;
    return applied;
}

bool visibleToCustomer(const Json& data, const Json& coupon, const std::string& userId) {
    if (!coupon.value("active", true)) return false;
    const std::string code = coupon.value("code", "");
    const std::int64_t limit = jsonx::integer(coupon, "usageLimit");
    if (limit > 0 && usedCount(data, code) >= limit) return false;
    if (coupon.value("oncePerCustomer", true) && usedBy(data, code, userId) > 0) return false;
    return true;
}

Json customerView(const Json& data, const Json& coupon) {
    const std::string code = coupon.value("code", "");
    const std::int64_t limit = jsonx::integer(coupon, "usageLimit");
    Json result{{"code", code},
                {"description", coupon.value("description", "")},
                {"percent", jsonx::integer(coupon, "percent")},
                {"minOrder", jsonx::integer(coupon, "minOrder")},
                {"oncePerCustomer", coupon.value("oncePerCustomer", true)}};
    result["remaining"] = limit > 0 ? Json(std::max<std::int64_t>(0, limit - usedCount(data, code))) : Json(nullptr);
    return result;
}

Json adminView(const Json& data, const Json& coupon) {
    const std::string code = coupon.value("code", "");
    return {{"code", code},
            {"description", coupon.value("description", "")},
            {"percent", jsonx::integer(coupon, "percent")},
            {"minOrder", jsonx::integer(coupon, "minOrder")},
            {"usageLimit", jsonx::integer(coupon, "usageLimit")},
            {"oncePerCustomer", coupon.value("oncePerCustomer", true)},
            {"active", coupon.value("active", true)},
            {"usedCount", usedCount(data, code)},
            {"discountGiven", discountGiven(data, code)},
            {"createdAt", coupon.value("createdAt", "")},
            {"updatedAt", coupon.value("updatedAt", coupon.value("createdAt", ""))}};
}

namespace {
std::int64_t bounded(const Json& body, const char* key, std::int64_t low, std::int64_t high, const char* label,
                     bool required, std::int64_t fallback) {
    if (!body.contains(key) || body[key].is_null()) {
        if (required) throw ApiError(400, std::string("Thiếu trường '") + key + "'.");
        return fallback;
    }
    const std::int64_t value = jsonx::integer(body, key);
    if (value < low || value > high) {
        throw ApiError(400, std::string(label) + " phải nằm trong khoảng " + std::to_string(low) + " – " +
                                std::to_string(high) + ".");
    }
    return value;
}
}  // namespace

Json parseFields(const Json& body) {
    const std::string description = jsonx::optionalString(body, "description", 200);
    return {{"description", description},
            {"percent", bounded(body, "percent", 1, kMaxPercent, "Phần trăm giảm", true, 0)},
            {"minOrder", bounded(body, "minOrder", 0, kMaxMinOrder, "Đơn hàng tối thiểu", false, 0)},
            {"usageLimit", bounded(body, "usageLimit", 0, kMaxUsageLimit, "Giới hạn lượt dùng (0 = không giới hạn)", false, 0)},
            {"oncePerCustomer", jsonx::optionalBool(body, "oncePerCustomer", true)},
            {"active", jsonx::optionalBool(body, "active", true)}};
}

}  // namespace shoppro::coupon
