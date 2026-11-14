#include "JsonStore.h"

#include <cstdio>
#include <fstream>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace shoppro {
namespace {

Json readJsonFile(const std::filesystem::path& file, const char* what) {
    std::ifstream input(file, std::ios::binary);
    if (!input) throw std::runtime_error(std::string("Không mở được ") + what + ": " + file.string());
    try {
        Json parsed;
        input >> parsed;
        return parsed;
    } catch (const Json::exception& error) {
        throw std::runtime_error(std::string(what) + " bị hỏng (" + file.string() + "): " + error.what() +
                                 ". Hãy khôi phục từ bản .bak cùng thư mục.");
    }
}

// Ghi tep va ep he dieu hanh ghi xuong dia truoc khi doi ten
void writeDurable(const std::filesystem::path& file, const std::string& content) {
#ifdef _WIN32
    FILE* handle = _wfopen(file.c_str(), L"wb");
#else
    FILE* handle = std::fopen(file.c_str(), "wb");
#endif
    if (!handle) throw std::runtime_error("Không tạo được tệp tạm: " + file.string());
    const bool written = std::fwrite(content.data(), 1, content.size(), handle) == content.size();
    const bool flushed = std::fflush(handle) == 0;
#ifdef _WIN32
    const bool synced = _commit(_fileno(handle)) == 0;
#else
    const bool synced = fsync(fileno(handle)) == 0;
#endif
    const bool closed = std::fclose(handle) == 0;
    if (!written || !flushed || !synced || !closed) throw std::runtime_error("Không ghi được tệp dữ liệu.");
}

}  // namespace

JsonStore::JsonStore(std::filesystem::path dataFile, std::filesystem::path seedFile)
    : dataFile_(std::move(dataFile)), seedFile_(std::move(seedFile)) {}

void JsonStore::shiftTimestamps(Json& node, std::int64_t delta) {
    if (node.is_object()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            const std::string& key = it.key();
            if (it.value().is_string() &&
                (key == "createdAt" || key == "updatedAt" || key == "joinedAt" || key == "at")) {
                const std::string value = it.value().get<std::string>();
                const std::int64_t parsed = timeutil::parseIso(value);
                if (parsed >= 0) {
                    const std::string shifted = timeutil::toIso(parsed + delta);
                    it.value() = value.size() == 10 ? shifted.substr(0, 10) : shifted;
                }
            } else {
                shiftTimestamps(it.value(), delta);
            }
        }
    } else if (node.is_array()) {
        for (auto& child : node) shiftTimestamps(child, delta);
    }
}

std::int64_t JsonStore::seedShiftSeconds() const {
    try {
        const Json seed = readJsonFile(seedFile_, "Tệp dữ liệu mẫu");
        const Json meta = seed.value("meta", Json::object());
        const std::int64_t reference = timeutil::parseIso(meta.value("referenceTime", ""));
        if (reference < 0) return 0;
        const std::int64_t delta = ((timeutil::epochSeconds() - reference) / 86400) * 86400;
        return delta > 0 ? delta : 0;
    } catch (const std::exception&) {
        return 0;
    }
}

Json JsonStore::load() {
    if (std::filesystem::exists(dataFile_)) {
        Json data = readJsonFile(dataFile_, "Tệp dữ liệu ShopPro");
        static const char* const required[] = {"users", "products", "categories",
                                               "orders", "carts", "settings"};
        for (const char* key : required) {
            if (!data.is_object() || !data.contains(key)) {
                throw std::runtime_error(std::string("Tệp dữ liệu thiếu khóa '") + key + "'.");
            }
        }
        return data;
    }

    Json seed = readJsonFile(seedFile_, "Tệp dữ liệu mẫu");
    if (!seed.is_object() || !seed.contains("products") || !seed.contains("orders") || !seed.contains("users")) {
        throw std::runtime_error("Dữ liệu mẫu không đầy đủ.");
    }
    // Dich thoi gian de don hang mau luon "moi" khi demo
    const std::int64_t delta = seedShiftSeconds();
    if (delta > 0) shiftTimestamps(seed, delta);
    seed.erase("meta");
    persist(seed);
    return seed;
}

void JsonStore::persist(const Json& data) const {
    const auto parent = dataFile_.parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent);

    auto temporary = dataFile_;
    temporary += ".tmp";
    writeDurable(temporary, data.dump(2, ' ', false) + "\n");

    // Sao luu ban truoc do (khong qua thuong xuyen)
    const std::int64_t now = timeutil::epochSeconds();
    std::error_code ignored;
    if (std::filesystem::exists(dataFile_, ignored) && now - lastBackup_ >= 60) {
        auto backup = dataFile_;
        backup += ".bak";
        std::filesystem::copy_file(dataFile_, backup, std::filesystem::copy_options::overwrite_existing, ignored);
        lastBackup_ = now;
    }

#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), dataFile_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        throw std::runtime_error("Không thay thế được tệp dữ liệu (MoveFileEx).");
    }
#else
    std::error_code error;
    std::filesystem::rename(temporary, dataFile_, error);
    if (error) throw std::runtime_error("Không thay thế được tệp dữ liệu: " + error.message());
#endif
}

}  // namespace shoppro
