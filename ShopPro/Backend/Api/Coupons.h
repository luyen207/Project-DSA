// Ma giam gia theo phan tram: kiem tra dieu kien (don toi thieu, gioi han luot, moi khach 1 lan),
// tinh so tien giam. Luot dung KHONG luu rieng: tinh tu cac don hang chua huy nen khong the lech.
#pragma once

#include "ApiContext.h"

namespace shoppro::coupon {

constexpr std::int64_t kMaxPercent = 90;                 // giam toi da 90% de don khong ve 0 dong
constexpr std::int64_t kMaxUsageLimit = 1000000;
constexpr std::int64_t kMaxMinOrder = kMaxProductPrice;  // 10 ty dong

// Cat khoang trang + doi chu HOA (ASCII). Khong kiem tra dinh dang.
std::string cleanCode(const std::string& raw);
// Ma hop le: 3-20 ky tu A-Z, 0-9, '-' hoac '_'
bool validCodeFormat(const std::string& code);

// floor(amount * percent / 100) khong tran so
std::int64_t percentOf(std::int64_t amount, std::int64_t percent);
// "1.250.000đ"
std::string formatMoney(std::int64_t amount);

const Json* find(const Json& data, const std::string& code);
Json* find(Json& data, const std::string& code);

// So don CHUA HUY da dung ma nay (cua moi khach / cua mot khach)
std::int64_t usedCount(const Json& data, const std::string& code);
std::int64_t usedBy(const Json& data, const std::string& code, const std::string& userId);
// Tong tien da giam cho cac don chua huy
std::int64_t discountGiven(const Json& data, const std::string& code);

struct Applied {
    std::string code;
    std::string description;
    std::int64_t percent = 0;
    std::int64_t minOrder = 0;
    std::int64_t subtotal = 0;
    std::int64_t discount = 0;
    std::int64_t total = 0;
};

// Kiem tra ma voi tong tien hang `subtotal` cua khach `userId`. Nem ApiError neu khong dung duoc:
// 400 (chua nhap), 404 (khong ton tai), 409 (tam ngung, het luot, da dung, chua du don toi thieu).
Applied apply(const Json& data, const std::string& rawCode, const std::string& userId, std::int64_t subtotal);

// Khach thay: ma dang hoat dong, con luot, chua dung (neu moi khach 1 lan). nullptr-json neu khong hien thi.
bool visibleToCustomer(const Json& data, const Json& coupon, const std::string& userId);
Json customerView(const Json& data, const Json& coupon);
Json adminView(const Json& data, const Json& coupon);

// Doc va kiem tra truong tu body Admin (dung cho tao moi va sua). Tra ve cac truong da chuan hoa (khong gom code).
Json parseFields(const Json& body);

}  // namespace shoppro::coupon
