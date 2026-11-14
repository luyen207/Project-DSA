#include "ProductManager.h"
#include "CartCalculator.h"
#include "ProductDisplay.h"

MaKetQua themSanPham(DanhSach &ds, int id, const std::string &ten, double gia, int soLuong) {
    MaKetQua kq = kiemTraDuLieuSanPham(id, ten, gia, soLuong);
    if (kq != THANH_CONG) {
        return kq;
    }

    // Truong hop san pham da ton tai: cong don so luong
    Node* daCo = timNodeTheoID(ds, id);
    if (daCo != nullptr) {
        if (daCo->soLuong > SO_LUONG_TOI_DA - soLuong) {   // tranh vuot tran
            return VUOT_SO_LUONG_TOI_DA;
        }
        daCo->soLuong += soLuong;
        return DA_CONG_DON;
    }

    // Truong hop san pham moi
    Node* nodeMoi = taoNode(id, ten, gia, soLuong);
    if (nodeMoi == nullptr) {
        return LOI_BO_NHO;
    }
    if (!themCuoi(ds, nodeMoi)) {
        delete nodeMoi;
        return LOI_BO_NHO;
    }
    return THANH_CONG;
}

MaKetQua xoaSanPhamTheoID(DanhSach &ds, int id) {
    if (!laIDHopLe(id)) {
        return ID_KHONG_HOP_LE;
    }
    if (!xoaTheoID(ds, id)) {
        return ID_KHONG_TON_TAI;
    }
    return THANH_CONG;
}

MaKetQua capNhatThongTin(DanhSach &ds, int id, const std::string &tenMoi, double giaMoi) {
    if (!laIDHopLe(id))      return ID_KHONG_HOP_LE;
    if (!laTenHopLe(tenMoi)) return TEN_KHONG_HOP_LE;
    if (!laGiaHopLe(giaMoi)) return GIA_KHONG_HOP_LE;

    Node* p = timNodeTheoID(ds, id);
    if (p == nullptr) {
        return ID_KHONG_TON_TAI;
    }
    p->ten = tenMoi;
    p->gia = giaMoi;
    return THANH_CONG;
}

MaKetQua capNhatSoLuong(DanhSach &ds, int id, int soLuongMoi) {
    if (!laIDHopLe(id))                return ID_KHONG_HOP_LE;
    if (!laSoLuongHopLe(soLuongMoi))   return SO_LUONG_KHONG_HOP_LE;

    Node* p = timNodeTheoID(ds, id);
    if (p == nullptr) {
        return ID_KHONG_TON_TAI;
    }
    p->soLuong = soLuongMoi;
    return THANH_CONG;
}

// Dung lai code cua TV4 / TV3, khong viet trung
double tinhTongTien(const DanhSach &ds) {
    return CartCalculator::tinhTongTien(ds);
}

void hienThiGioHang(const DanhSach &ds) {
    ProductDisplay::hienThiTatCa(ds, true);
}
