// Kho du lieu JSON: doc, ghi nguyen tu, sao luu, nap du lieu mau.
#pragma once

#include <filesystem>
#include <mutex>

#include "../Support.h"

namespace shoppro {

class JsonStore {
public:
    // dataFile: tep du lieu chay that (vd data/shoppro.json)
    // seedFile: du lieu mau, chi dung khi dataFile chua ton tai
    JsonStore(std::filesystem::path dataFile, std::filesystem::path seedFile);

    // Doc tep du lieu; chua co thi nap tu seed (dich moc thoi gian ve "bay gio") va luu lai.
    Json load();

    // Ghi nguyen tu: ghi .tmp -> fsync -> doi ten de thay the. Giu ban .bak (toi da 1 lan / 60 giay).
    void persist(const Json& data) const;

    const std::filesystem::path& dataFile() const { return dataFile_; }
    const std::filesystem::path& seedFile() const { return seedFile_; }

    // So giay (boi so cua 1 ngay) can dich du lieu mau de moc "referenceTime" thanh hom nay. 0 neu khong can.
    std::int64_t seedShiftSeconds() const;

    // Dich cac truong thoi gian (createdAt, updatedAt, joinedAt, at) cua seed them deltaSeconds.
    static void shiftTimestamps(Json& node, std::int64_t deltaSeconds);

private:
    std::filesystem::path dataFile_;
    std::filesystem::path seedFile_;
    mutable std::int64_t lastBackup_ = 0;
};

}  // namespace shoppro
