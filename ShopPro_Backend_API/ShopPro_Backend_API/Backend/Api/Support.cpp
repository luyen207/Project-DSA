#include "Support.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <map>
#include <regex>
#include <sstream>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace shoppro {

// =============================== text ===============================
namespace text {
namespace {

// Giai ma 1 ky tu UTF-8 tai vi tri index; tra ve do rong (1-4 byte)
std::size_t decode(const std::string& s, std::size_t index, std::uint32_t& cp) {
    const auto first = static_cast<unsigned char>(s[index]);
    cp = first;
    if ((first & 0xE0) == 0xC0 && index + 1 < s.size()) {
        cp = ((first & 0x1Fu) << 6) | (static_cast<unsigned char>(s[index + 1]) & 0x3Fu);
        return 2;
    }
    if ((first & 0xF0) == 0xE0 && index + 2 < s.size()) {
        cp = ((first & 0x0Fu) << 12) | ((static_cast<unsigned char>(s[index + 1]) & 0x3Fu) << 6) |
             (static_cast<unsigned char>(s[index + 2]) & 0x3Fu);
        return 3;
    }
    if ((first & 0xF8) == 0xF0 && index + 3 < s.size()) {
        cp = ((first & 0x07u) << 18) | ((static_cast<unsigned char>(s[index + 1]) & 0x3Fu) << 12) |
             ((static_cast<unsigned char>(s[index + 2]) & 0x3Fu) << 6) |
             (static_cast<unsigned char>(s[index + 3]) & 0x3Fu);
        return 4;
    }
    return 1;
}

void encode(std::string& out, std::uint32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

const std::map<std::uint32_t, char>& foldTable() {
    static const std::map<std::uint32_t, char> table = [] {
        std::map<std::uint32_t, char> result;
        const auto add = [&](const std::string& letters, char base) {
            for (std::size_t i = 0; i < letters.size();) {
                std::uint32_t cp = 0;
                i += decode(letters, i, cp);
                result[cp] = base;
            }
        };
        add("àáạảãâầấậẩẫăằắặẳẵ", 'a');
        add("èéẹẻẽêềếệểễ", 'e');
        add("ìíịỉĩ", 'i');
        add("òóọỏõôồốộổỗơờớợởỡ", 'o');
        add("ùúụủũưừứựửữ", 'u');
        add("ỳýỵỷỹ", 'y');
        add("đ", 'd');
        return result;
    }();
    return table;
}

}  // namespace

std::string lower(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t index = 0; index < value.size();) {
        std::uint32_t cp = 0;
        const std::size_t width = decode(value, index, cp);
        if (cp >= 'A' && cp <= 'Z') {
            cp += 32;
        } else if ((cp >= 0x00C0 && cp <= 0x00D6) || (cp >= 0x00D8 && cp <= 0x00DE)) {
            cp += 32;
        } else if ((cp >= 0x0100 && cp <= 0x012F && cp % 2 == 0) ||
                   (cp >= 0x0132 && cp <= 0x0137 && cp % 2 == 0) ||
                   (cp == 0x0168) ||                                  // Ũ
                   (cp == 0x01A0) ||                                  // Ơ
                   (cp == 0x01AF) ||                                  // Ư
                   (cp >= 0x1EA0 && cp <= 0x1EF8 && cp % 2 == 0)) {
            ++cp;
        } else if (cp == 0x0110) {
            cp = 0x0111;   // Đ -> đ
        }
        encode(result, cp);
        index += width;
    }
    return result;
}

std::string foldVietnamese(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    const auto& table = foldTable();
    for (std::size_t index = 0; index < value.size();) {
        std::uint32_t cp = 0;
        const std::size_t width = decode(value, index, cp);
        const auto found = table.find(cp);
        if (found != table.end()) result.push_back(found->second);
        else result.append(value, index, width);
        index += width;
    }
    return result;
}

std::string searchKey(const std::string& value) { return foldVietnamese(lower(value)); }

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string urlDecode(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    const auto hex = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '+') {
            result.push_back(' ');
        } else if (value[i] == '%') {
            if (i + 2 >= value.size()) throw ApiError(400, "URL không hợp lệ.");
            const int high = hex(value[i + 1]);
            const int low = hex(value[i + 2]);
            if (high < 0 || low < 0) throw ApiError(400, "URL không hợp lệ.");
            result.push_back(static_cast<char>((high << 4) | low));
            i += 2;
        } else {
            result.push_back(value[i]);
        }
    }
    return result;
}

std::vector<std::string> split(const std::string& value, char delimiter) {
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string part;
    while (std::getline(stream, part, delimiter)) result.push_back(part);
    return result;
}

bool validEmail(const std::string& email) {
    static const std::regex pattern(R"(^[^@\s]{1,64}@[^@\s.]+(?:\.[^@\s.]+)+$)");
    return email.size() <= 160 && std::regex_match(email, pattern);
}

bool validPhone(const std::string& phone) {
    int digits = 0;
    for (const char ch : phone) {
        if (ch >= '0' && ch <= '9') ++digits;
        else if (ch != ' ' && ch != '+' && ch != '-' && ch != '.' && ch != '(' && ch != ')') return false;
    }
    return digits >= 8 && digits <= 15;
}

bool validUtf8(const std::string& value) {
    for (std::size_t i = 0; i < value.size();) {
        const auto c = static_cast<unsigned char>(value[i]);
        std::size_t extra = 0;
        if (c < 0x80) extra = 0;
        else if ((c & 0xE0) == 0xC0 && c >= 0xC2) extra = 1;
        else if ((c & 0xF0) == 0xE0) extra = 2;
        else if ((c & 0xF8) == 0xF0 && c <= 0xF4) extra = 3;
        else return false;
        if (extra > 0 && i + extra >= value.size()) return false;
        for (std::size_t k = 1; k <= extra; ++k) {
            if ((static_cast<unsigned char>(value[i + k]) & 0xC0) != 0x80) return false;
        }
        i += extra + 1;
    }
    return true;
}

}  // namespace text

// ============================== timeutil ==============================
namespace timeutil {
namespace {

std::int64_t daysFromCivil(std::int64_t y, unsigned m, unsigned d) {
    y -= m <= 2;
    const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<std::int64_t>(doe) - 719468;
}

void civilFromDays(std::int64_t z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    if (m <= 2) ++y;
}

}  // namespace

std::int64_t epochSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string toIso(std::int64_t seconds) {
    std::int64_t days = seconds / 86400;
    std::int64_t rest = seconds % 86400;
    if (rest < 0) { rest += 86400; --days; }
    int y = 0; unsigned m = 0, d = 0;
    civilFromDays(days, y, m, d);
    char buffer[40];
    std::snprintf(buffer, sizeof buffer, "%04d-%02u-%02uT%02d:%02d:%02d.000Z", y, m, d,
                  static_cast<int>(rest / 3600), static_cast<int>((rest % 3600) / 60),
                  static_cast<int>(rest % 60));
    return buffer;
}

std::string nowIso() { return toIso(epochSeconds()); }
std::string todayIso() { return nowIso().substr(0, 10); }

std::int64_t parseIso(const std::string& iso) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (iso.size() >= 19 &&
        std::sscanf(iso.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &y, &mo, &d, &h, &mi, &s) == 6) {
        return daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d)) * 86400 + h * 3600 + mi * 60 + s;
    }
    if (iso.size() >= 10 && std::sscanf(iso.c_str(), "%4d-%2d-%2d", &y, &mo, &d) == 3) {
        return daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d)) * 86400;
    }
    return -1;
}

}  // namespace timeutil

// =============================== crypto ===============================
namespace crypto {

std::string randomHex(std::size_t byteCount) {
    std::vector<unsigned char> bytes(byteCount);
    if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1) {
        throw ApiError(500, "Không thể sinh số ngẫu nhiên an toàn.");
    }
    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (const unsigned char byte : bytes) result << std::setw(2) << static_cast<unsigned int>(byte);
    return result.str();
}

std::string hashPassword(const std::string& password, const std::string& salt, int iterations) {
    std::array<unsigned char, 32> output{};
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
                          reinterpret_cast<const unsigned char*>(salt.data()), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), static_cast<int>(output.size()), output.data()) != 1) {
        throw ApiError(500, "Băm mật khẩu thất bại.");
    }
    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (const unsigned char byte : output) result << std::setw(2) << static_cast<unsigned int>(byte);
    return result.str();
}

bool verifyPassword(const std::string& password, const Json& user) {
    const std::string salt = user.value("passwordSalt", "");
    const std::string expected = user.value("passwordHash", "");
    const auto iterations = static_cast<int>(jsonx::integer(user, "passwordIterations", kPasswordIterations));
    if (salt.empty() || expected.size() != 64 || iterations < 1000 || iterations > 5000000) return false;
    const std::string actual = hashPassword(password, salt, iterations);
    return CRYPTO_memcmp(actual.data(), expected.data(), expected.size()) == 0;
}

}  // namespace crypto

// ================================ jsonx ================================
namespace jsonx {

std::int64_t integer(const Json& value, const std::string& key, std::int64_t fallback) {
    if (!value.is_object() || !value.contains(key) || value[key].is_null()) return fallback;
    const Json& field = value[key];
    if (field.is_number_integer()) return field.get<std::int64_t>();
    if (field.is_number()) {
        const double number = field.get<double>();
        if (!std::isfinite(number) || std::floor(number) != number ||
            number > static_cast<double>(std::numeric_limits<std::int64_t>::max()) ||
            number < static_cast<double>(std::numeric_limits<std::int64_t>::min())) {
            throw ApiError(400, "Trường '" + key + "' phải là số nguyên.");
        }
        return static_cast<std::int64_t>(number);
    }
    throw ApiError(400, "Trường '" + key + "' phải là số nguyên.");
}

std::int64_t queryInteger(const std::map<std::string, std::string>& query, const std::string& key,
                          std::int64_t fallback) {
    const auto found = query.find(key);
    if (found == query.end() || found->second.empty()) return fallback;
    try {
        std::size_t consumed = 0;
        const auto value = std::stoll(found->second, &consumed);
        if (consumed != found->second.size()) throw std::invalid_argument("tail");
        return value;
    } catch (const std::exception&) {
        throw ApiError(400, "Tham số '" + key + "' phải là số nguyên.");
    }
}

std::string requiredString(const Json& value, const std::string& key, std::size_t maxLength) {
    if (!value.is_object() || !value.contains(key) || !value[key].is_string()) {
        throw ApiError(400, "Thiếu trường '" + key + "'.");
    }
    const std::string result = text::trim(value[key].get<std::string>());
    if (result.empty() || result.size() > maxLength) throw ApiError(400, "Trường '" + key + "' không hợp lệ.");
    return result;
}

std::string optionalString(const Json& value, const std::string& key, std::size_t maxLength) {
    if (!value.is_object() || !value.contains(key) || value[key].is_null()) return {};
    if (!value[key].is_string()) throw ApiError(400, "Trường '" + key + "' phải là chuỗi.");
    const std::string result = text::trim(value[key].get<std::string>());
    if (result.size() > maxLength) throw ApiError(400, "Trường '" + key + "' quá dài.");
    return result;
}

bool optionalBool(const Json& value, const std::string& key, bool fallback) {
    if (!value.is_object() || !value.contains(key) || value[key].is_null()) return fallback;
    if (!value[key].is_boolean()) throw ApiError(400, "Trường '" + key + "' phải là true/false.");
    return value[key].get<bool>();
}

Json parseObject(const std::string& body) {
    if (body.empty()) return Json::object();
    if (!text::validUtf8(body)) throw ApiError(400, "Nội dung yêu cầu không phải UTF-8 hợp lệ.");
    try {
        Json result = Json::parse(body);
        if (!result.is_object()) throw ApiError(400, "Nội dung yêu cầu phải là một đối tượng JSON.");
        return result;
    } catch (const ApiError&) {
        throw;
    } catch (const Json::exception&) {
        throw ApiError(400, "Nội dung yêu cầu không phải JSON hợp lệ.");
    }
}

const Json* findById(const Json& array, std::int64_t id) {
    if (!array.is_array()) return nullptr;
    for (const auto& item : array) {
        if (item.is_object() && item.contains("id") && item["id"].is_number_integer() &&
            item["id"].get<std::int64_t>() == id) return &item;
    }
    return nullptr;
}

Json* findById(Json& array, std::int64_t id) {
    return const_cast<Json*>(findById(static_cast<const Json&>(array), id));
}

const Json* findByStringId(const Json& array, const std::string& id) {
    if (!array.is_array()) return nullptr;
    for (const auto& item : array) {
        if (item.is_object() && item.contains("id") && item["id"].is_string() &&
            item["id"].get<std::string>() == id) return &item;
    }
    return nullptr;
}

Json* findByStringId(Json& array, const std::string& id) {
    return const_cast<Json*>(findByStringId(static_cast<const Json&>(array), id));
}

}  // namespace jsonx
}  // namespace shoppro
