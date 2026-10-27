#pragma once

#include <string>
#include <vector>
#include "MatHang.h"

struct NutMatHang {
    MatHang matHang;
    NutMatHang* truoc;
    NutMatHang* sau;

    explicit NutMatHang(const MatHang& hang)
        : matHang(hang), truoc(nullptr), sau(nullptr) {}
};

class DanhSachLienKetDoi {
public:
    DanhSachLienKetDoi() = default;
    ~DanhSachLienKetDoi();

    DanhSachLienKetDoi(const DanhSachLienKetDoi&) = delete;
    DanhSachLienKetDoi& operator=(const DanhSachLienKetDoi&) = delete;

    int soNut() const;
    bool rong() const;
    NutMatHang* layDau();
    const NutMatHang* layDau() const;
    NutMatHang* layCuoi();
    const NutMatHang* layCuoi() const;
    NutMatHang* timTheoMa(const std::string& ma);
    const NutMatHang* timTheoMa(const std::string& ma) const;
    KetQua themCuoi(const MatHang& matHang);
    KetQua xoaTheoMa(const std::string& ma);
    void xoaTatCa();
    void saoChepTu(const DanhSachLienKetDoi& danhSach);
    std::vector<MatHang> layDanhSach(bool xuoi = true) const;

private:
    NutMatHang* dau_ = nullptr;
    NutMatHang* cuoi_ = nullptr;
    int soNut_ = 0;
};
