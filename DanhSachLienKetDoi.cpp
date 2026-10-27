#include "DanhSachLienKetDoi.h"

DanhSachLienKetDoi::~DanhSachLienKetDoi() {
    xoaTatCa();
}

int DanhSachLienKetDoi::soNut() const {
    return soNut_;
}

bool DanhSachLienKetDoi::rong() const {
    return dau_ == nullptr;
}

NutMatHang* DanhSachLienKetDoi::layDau() {
    return dau_;
}

const NutMatHang* DanhSachLienKetDoi::layDau() const {
    return dau_;
}

NutMatHang* DanhSachLienKetDoi::layCuoi() {
    return cuoi_;
}

const NutMatHang* DanhSachLienKetDoi::layCuoi() const {
    return cuoi_;
}

NutMatHang* DanhSachLienKetDoi::timTheoMa(const std::string& ma) {
    for (NutMatHang* nut = dau_; nut != nullptr; nut = nut->sau) {
        if (nut->matHang.maHang == ma) return nut;
    }
    return nullptr;
}

const NutMatHang* DanhSachLienKetDoi::timTheoMa(const std::string& ma) const {
    for (const NutMatHang* nut = dau_; nut != nullptr; nut = nut->sau) {
        if (nut->matHang.maHang == ma) return nut;
    }
    return nullptr;
}

KetQua DanhSachLienKetDoi::themCuoi(const MatHang& matHang) {
    if (soNut_ >= GIO_TOAN_DUNG_TOI_DA) return KetQua::SO_LUONG_KHONG_HOP_LE;

    NutMatHang* nutMoi = new NutMatHang(matHang);
    nutMoi->truoc = cuoi_;
    if (cuoi_ != nullptr) {
        cuoi_->sau = nutMoi;
    } else {
        dau_ = nutMoi;
    }
    cuoi_ = nutMoi;
    ++soNut_;
    return KetQua::THANH_CONG;
}

KetQua DanhSachLienKetDoi::xoaTheoMa(const std::string& ma) {
    NutMatHang* nut = timTheoMa(ma);
    if (nut == nullptr) return KetQua::KHONG_TIM_THAY;

    if (nut->truoc != nullptr) nut->truoc->sau = nut->sau;
    else dau_ = nut->sau;
    if (nut->sau != nullptr) nut->sau->truoc = nut->truoc;
    else cuoi_ = nut->truoc;

    delete nut;
    --soNut_;
    return KetQua::THANH_CONG;
}

void DanhSachLienKetDoi::xoaTatCa() {
    NutMatHang* nut = dau_;
    while (nut != nullptr) {
        NutMatHang* tiepTheo = nut->sau;
        delete nut;
        nut = tiepTheo;
    }
    dau_ = nullptr;
    cuoi_ = nullptr;
    soNut_ = 0;
}

void DanhSachLienKetDoi::saoChepTu(const DanhSachLienKetDoi& danhSach) {
    if (this == &danhSach) return;

    DanhSachLienKetDoi banSao;
    for (const NutMatHang* nut = danhSach.dau_; nut != nullptr; nut = nut->sau) {
        banSao.themCuoi(nut->matHang);
    }

    xoaTatCa();
    dau_ = banSao.dau_;
    cuoi_ = banSao.cuoi_;
    soNut_ = banSao.soNut_;
    banSao.dau_ = nullptr;
    banSao.cuoi_ = nullptr;
    banSao.soNut_ = 0;
}

std::vector<MatHang> DanhSachLienKetDoi::layDanhSach(bool xuoi) const {
    std::vector<MatHang> ketQua;
    ketQua.reserve(static_cast<std::size_t>(soNut_));
    const NutMatHang* nut = xuoi ? dau_ : cuoi_;
    while (nut != nullptr) {
        ketQua.push_back(nut->matHang);
        nut = xuoi ? nut->sau : nut->truoc;
    }
    return ketQua;
}
