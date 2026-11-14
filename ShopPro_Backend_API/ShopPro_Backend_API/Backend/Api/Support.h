// Tien ich dung chung cho tang REST: loi API, xu ly chuoi UTF-8, thoi gian,
// mat khau (PBKDF2) va doc/kiem tra JSON dau vao.
#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace shoppro {

using Json = nlohmann::json;

// ---- Gioi han nghiep vu (kiem tra o server, khong tin Frontend) ----
constexpr std::int64_t kMaxProductPrice = 10000000000LL;      // 10 ty dong / san pham
constexpr std::int64_t kMaxStock = 100000;                    // ton kho toi da / san pham
constexpr std::int64_t kMaxSafeTotal = 9007199254740991LL;    // 2^53 - 1
constexpr int kPasswordIterations = 600000;                   // PBKDF2-HMAC-SHA256

// Loi nghiep vu: duoc dich thanh HTTP status + {"error": "..."}
struct ApiError : std::runtime_error {
    int status;
    int retryAfterSeconds;
    ApiError(int statusCode, const std::string& message, int retryAfter = 0)
        : std::runtime_error(message), status(statusCode), retryAfterSeconds(retryAfter) {}
};

namespace text {
std::string lower(const std::string& value);          // chu thuong UTF-8 (ho tro tieng Viet)
std::string foldVietnamese(const std::string& value); // bo dau tieng Viet (da chuyen chu thuong)
std::string searchKey(const std::string& value);      // lower + bo dau, dung de tim kiem
std::string trim(const std::string& value);
std::string urlDecode(const std::string& value);
std::vector<std::string> split(const std::string& value, char delimiter);
bool validEmail(const std::string& email);
bool validPhone(const std::string& phone);
bool validUtf8(const std::string& value);
}  // namespace text

namespace timeutil {
std::string nowIso();                              // 2026-11-14T08:30:00.000Z (UTC)
std::string todayIso();                            // 2026-11-14
std::int64_t epochSeconds();
std::int64_t parseIso(const std::string& iso);     // -1 neu sai dinh dang
std::string toIso(std::int64_t epochSeconds);
}  // namespace timeutil

namespace crypto {
std::string randomHex(std::size_t byteCount);
std::string hashPassword(const std::string& password, const std::string& salt, int iterations);
// So sanh thoi gian khong doi. iterations lay tu ban ghi da luu cua user.
bool verifyPassword(const std::string& password, const Json& user);
}  // namespace crypto

namespace jsonx {
std::int64_t integer(const Json& value, const std::string& key, std::int64_t fallback = 0);
std::int64_t queryInteger(const std::map<std::string, std::string>& query, const std::string& key,
                          std::int64_t fallback);
std::string requiredString(const Json& value, const std::string& key, std::size_t maxLength = 500);
std::string optionalString(const Json& value, const std::string& key, std::size_t maxLength = 2000);
bool optionalBool(const Json& value, const std::string& key, bool fallback);
Json parseObject(const std::string& body);

const Json* findById(const Json& array, std::int64_t id);
Json* findById(Json& array, std::int64_t id);
const Json* findByStringId(const Json& array, const std::string& id);
Json* findByStringId(Json& array, const std::string& id);
}  // namespace jsonx

}  // namespace shoppro
