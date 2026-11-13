#include "CartController.h"

#include <ostream>

#include "CartCalculator.h"
#include "ProductDisplay.h"
#include "ProductManager.h"
#include "ProductSearch.h"
#include "ProductSort.h"

MaKetQua CartController::themSanPham(int id, const std::string& ten, double gia, int soLuong) {
    return ::themSanPham(ds, id, ten, gia, soLuong);
}

MaKetQua CartController::xoaSanPham(int id) {
    return xoaSanPhamTheoID(ds, id);
}

MaKetQua CartController::capNhatThongTin(int id, const std::string& ten, double gia) {
    return ::capNhatThongTin(ds, id, ten, gia);
}

MaKetQua CartController::capNhatSoLuong(int id, int soLuong) {
    return ::capNhatSoLuong(ds, id, soLuong);
}

void CartController::lamTrongGio() {
    giaiPhongDanhSach(ds);
}

const Node* CartController::timTheoId(int id) const {
    return ProductSearch::timTheoId(ds, id);
}

std::vector<const Node*> CartController::timTheoTen(const std::string& tuKhoa) const {
    return ProductSearch::timTheoTen(ds, tuKhoa);
}

void CartController::hienThiTatCa(bool xuoi, std::ostream& out) const {
    ProductDisplay::hienThiTatCa(ds, xuoi, out);
}

void CartController::hienThiKetQuaTimId(int id, std::ostream& out) const {
    ProductDisplay::hienThiMotSanPham(timTheoId(id), id, out);
}

void CartController::hienThiKetQuaTimTen(const std::string& tuKhoa, std::ostream& out) const {
    std::vector<const Node*> ketQua = timTheoTen(tuKhoa);
    if (ketQua.empty()) {
        ProductDisplay::hienThiKhongTimThayTen(tuKhoa, out);
    } else {
        ProductDisplay::hienThiDanhSach(ketQua, out);
    }
}

void CartController::sapXep(KieuSapXep kieu) {
    if (kieu == GIA_TANG) {
        ProductSort::sapXepTheoGiaTangDan(ds);
    } else if (kieu == GIA_GIAM) {
        ProductSort::sapXepTheoGiaGiamDan(ds);
    } else if (kieu == THEO_TEN) {
        ProductSort::sapXepTheoTen(ds);
    } else {
        ProductSort::sapXepTheoID(ds);
    }
}

double CartController::tongTien() const {
    return CartCalculator::tinhTongTien(ds);
}

long long CartController::tongSoLuong() const {
    return CartCalculator::tinhTongSoLuong(ds);
}

int CartController::soLoaiSanPham() const {
    return CartCalculator::demSoLoaiSanPham(ds);
}

bool CartController::giaRong() const {
    return laRong(ds);
}

bool CartController::lienKetHopLe() const {
    return kiemTraLienKet(ds);
}
