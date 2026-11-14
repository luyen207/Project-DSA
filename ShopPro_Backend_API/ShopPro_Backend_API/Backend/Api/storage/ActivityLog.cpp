#include "ActivityLog.h"

#include <fstream>
#include <iostream>

#include "JsonStore.h"

namespace shoppro {

ActivityLog::ActivityLog(std::filesystem::path file, std::filesystem::path seedFile, std::int64_t seedShiftSeconds)
    : file_(std::move(file)) {
    const bool existed = std::filesystem::exists(file_);
    const std::filesystem::path source = existed ? file_ : seedFile;
    std::ifstream input(source, std::ios::binary);
    if (input) {
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) continue;
            try {
                Json entry = Json::parse(line);
                if (!entry.is_object()) continue;
                if (!existed && seedShiftSeconds > 0) JsonStore::shiftTimestamps(entry, seedShiftSeconds);
                entries_.push_back(std::move(entry));
                if (entries_.size() > kMaxInMemory) entries_.pop_front();
            } catch (const Json::exception&) {
                // bo qua dong hong, khong lam hong ca nhat ky
            }
        }
    }
    if (!existed && !entries_.empty()) {
        // luu ban mau da dich thoi gian thanh nhat ky that
        std::error_code ignored;
        if (!file_.parent_path().empty()) std::filesystem::create_directories(file_.parent_path(), ignored);
        std::ofstream output(file_, std::ios::binary | std::ios::trunc);
        for (const auto& entry : entries_) output << entry.dump(-1, ' ', false) << '\n';
    }
}

void ActivityLog::append(const Json& entry) {
    try {
        std::error_code ignored;
        if (!file_.parent_path().empty()) std::filesystem::create_directories(file_.parent_path(), ignored);
        std::ofstream output(file_, std::ios::binary | std::ios::app);
        if (!output) throw std::runtime_error("open");
        output << entry.dump(-1, ' ', false) << '\n';
        output.flush();
    } catch (const std::exception&) {
        // Nhat ky loi khong duoc lam hong yeu cau cua nguoi dung
        std::cerr << "[canh bao] khong ghi duoc nhat ky hoat dong\n";
    }
}

void ActivityLog::record(const std::string& action, const std::string& actorId, const std::string& actorRole,
                         const std::string& targetType, const std::string& targetId, const std::string& result,
                         const Json& details, const std::string& ip) {
    Json entry{{"id", "log-" + crypto::randomHex(6)},
               {"createdAt", timeutil::nowIso()},
               {"action", action},
               {"actorId", actorId},
               {"actorRole", actorRole},
               {"targetType", targetType},
               {"targetId", targetId},
               {"result", result}};
    if (details.is_object() && !details.empty()) entry["details"] = details;
    if (!ip.empty()) entry["ip"] = ip;

    std::lock_guard<std::mutex> lock(mutex_);
    append(entry);
    entries_.push_back(std::move(entry));
    if (entries_.size() > kMaxInMemory) entries_.pop_front();
}

std::size_t ActivityLog::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

Json ActivityLog::query(const LogQuery& q) const {
    const int page = q.page < 1 ? 1 : q.page;
    const int limit = q.limit < 1 ? 1 : (q.limit > 200 ? 200 : q.limit);
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<const Json*> matched;
    for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {   // moi nhat truoc
        const Json& entry = *it;
        if (!q.action.empty() && entry.value("action", "") != q.action) continue;
        if (!q.actorId.empty() && entry.value("actorId", "") != q.actorId) continue;
        if (!q.result.empty() && entry.value("result", "") != q.result) continue;
        const std::string day = entry.value("createdAt", "").substr(0, 10);
        if (!q.from.empty() && day < q.from) continue;
        if (!q.to.empty() && day > q.to) continue;
        matched.push_back(&entry);
    }
    Json items = Json::array();
    const std::size_t start = static_cast<std::size_t>(page - 1) * static_cast<std::size_t>(limit);
    for (std::size_t i = start; i < matched.size() && i < start + static_cast<std::size_t>(limit); ++i) {
        items.push_back(*matched[i]);
    }
    return {{"items", items}, {"page", page}, {"limit", limit}, {"total", matched.size()}};
}

}  // namespace shoppro
