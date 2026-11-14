// Nhat ky hoat dong (Activity Log): moi su kien la mot dong JSON trong tep .jsonl
// (chi noi them cuoi tep). Khong bao gio ghi mat khau, hash hay token.
#pragma once

#include <deque>
#include <filesystem>
#include <mutex>

#include "../Support.h"

namespace shoppro {

struct LogQuery {
    int page = 1;
    int limit = 50;
    std::string action;      // loc theo ma hanh dong (vd "order.placed")
    std::string actorId;
    std::string result;      // success | failure | blocked
    std::string from;        // YYYY-MM-DD (bao gom)
    std::string to;          // YYYY-MM-DD (bao gom)
};

class ActivityLog {
public:
    // seedFile: nhat ky mau (jsonl), chi nap khi tep nhat ky chua ton tai
    ActivityLog(std::filesystem::path file, std::filesystem::path seedFile, std::int64_t seedShiftSeconds);

    void record(const std::string& action, const std::string& actorId, const std::string& actorRole,
                const std::string& targetType, const std::string& targetId, const std::string& result,
                const Json& details = Json::object(), const std::string& ip = "");

    Json query(const LogQuery& query) const;   // {items, page, limit, total}, moi nhat truoc
    std::size_t size() const;

private:
    void append(const Json& entry);

    std::filesystem::path file_;
    mutable std::mutex mutex_;
    std::deque<Json> entries_;                 // tu cu den moi
    static constexpr std::size_t kMaxInMemory = 20000;
};

}  // namespace shoppro
