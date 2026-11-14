// Chuyen ban ghi luu tru -> JSON tra ve cho Frontend (khong bao gio lo hash/salt).
#pragma once

#include "../ApiContext.h"

namespace shoppro::view {

const Json* findCategory(const Json& data, std::int64_t id);
const Json* findProduct(const Json& data, std::int64_t id);
Json* findProduct(Json& data, std::int64_t id);

Json publicProduct(const Json& data, const Json& product);
Json adminProduct(const Json& data, const Json& product);
Json adminCategory(const Json& data, const Json& category);
Json adminOrder(const Json& order);
Json customerOrder(const Json& data, const Json& order);
Json publicUser(const Json& user);
Json adminUser(const Json& user);
Json adminCustomer(const Json& data, const Json& user);   // kem so don, tong chi tieu

// Gio hang cua user: dong bo voi danh muc roi tra ve {items,total,...,dsa}
Json cartPayload(Ctx& ctx, const std::string& userId);

// Cong don an toan: nem ApiError(400) neu vuot gioi han so nguyen an toan cua JSON/JS
std::int64_t checkedMultiply(std::int64_t price, std::int64_t quantity);
std::int64_t checkedAdd(std::int64_t total, std::int64_t amount);

// Ma don hang khong trung: SP-YYMMDD-XXXX
std::string newOrderId(const Json& orders);

// Huy don: tra ton kho cho cac san pham con ton tai (nem ApiError 409 neu vuot tran ton kho)
void restoreStockOnCancel(Json& data, Json& order);

bool isTerminalStatus(const std::string& status);
bool allowedTransition(const std::string& current, const std::string& next);

}  // namespace shoppro::view
