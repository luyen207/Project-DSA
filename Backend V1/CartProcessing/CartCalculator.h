#pragma once

#include "DoublyLinkedList.h"

// Tinh toan cac chi tieu tong hop cua gio hang
// Chi doc danh sach, khong sua doi

namespace CartCalculator {

// Tinh tong tien cua toan bo gio hang: sum(gia * soLuong)
// Gio rong tra ve 0
double tinhTongTien(const DanhSach& ds);

// Tinh tong so luong san pham trong gio: sum(soLuong)
// Gio rong tra ve 0. Dung long long de khong tran khi gio co rat nhieu dong
long long tinhTongSoLuong(const DanhSach& ds);

// Dem so loai san pham (so node) trong gio hang
// Gio rong tra ve 0
int demSoLoaiSanPham(const DanhSach& ds);

// Tinh gia trung binh cua cac san pham: tong tien / tong so luong
// Gio rong hoac tong so luong = 0 tra ve 0
double tinhGiaTrungBinh(const DanhSach& ds);

}
