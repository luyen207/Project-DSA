#include "CartCalculator.h"

namespace CartCalculator {

double tinhTongTien(const DanhSach& ds) {
    if (laRong(ds)) {
        return 0.0;
    }

    double tong = 0.0;
    int soBuoc = 0;

    // Duyet xuoi, gioi han soNode buoc de phong truoc lien ket hong
    for (const Node* p = ds.head; p != nullptr && soBuoc < ds.soNode; p = p->next, ++soBuoc) {
        tong += p->gia * static_cast<double>(p->soLuong);
    }

    return tong;
}

long long tinhTongSoLuong(const DanhSach& ds) {
    if (laRong(ds)) {
        return 0;
    }

    long long tong = 0;
    int soBuoc = 0;

    // Duyet xuoi
    for (const Node* p = ds.head; p != nullptr && soBuoc < ds.soNode; p = p->next, ++soBuoc) {
        tong += p->soLuong;   // int -> long long, khong tran
    }

    return tong;
}

int demSoLoaiSanPham(const DanhSach& ds) {
    return ds.soNode;
}

double tinhGiaTrungBinh(const DanhSach& ds) {
    long long tongSoLuong = tinhTongSoLuong(ds);
    if (tongSoLuong == 0) {
        return 0.0;
    }

    double tongTien = tinhTongTien(ds);
    return tongTien / static_cast<double>(tongSoLuong);
}

}
