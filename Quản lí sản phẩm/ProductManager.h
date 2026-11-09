#pragma once
#include <string>
#include "DoublyLinkedList.h"
#include "ProductValidation.h"
using namespace std;

// Them san pham vao CUOI gio hang.
//  - Du lieu sai            -> tra ve ma loi tuong ung, gio hang khong doi
//  - ID chua co             -> tao node moi, tra ve THANH_CONG
//  - ID da co               -> cong don soLuong vao node cu (giu nguyen ten, gia cu),
//                              tra ve DA_CONG_DON; neu tong vuot INT_MAX
//                              thi tra ve VUOT_SO_LUONG_TOI_DA va khong doi gi
MaKetQua themSanPham(DanhSach &ds, int id, const string &ten, double gia, int soLuong);

// Xoa san pham theo ID. Tra ve THANH_CONG, ID_KHONG_HOP_LE hoac ID_KHONG_TON_TAI.
MaKetQua xoaSanPhamTheoID(DanhSach &ds, int id);

// Cap nhat ten va gia cua san pham (khong doi id, khong doi so luong).
// Du lieu moi sai thi giu nguyen du lieu cu.
MaKetQua capNhatThongTin(DanhSach &ds, int id, const string &tenMoi, double giaMoi);

// Dat lai so luong (ghi de, khong cong don). soLuongMoi phai > 0;
// muon bo san pham khoi gio thi dung xoaSanPhamTheoID.
MaKetQua capNhatSoLuong(DanhSach &ds, int id, int soLuongMoi);

// Tong tien cua ca gio hang
double tinhTongTien(const DanhSach &ds);

// In gio hang ra man hinh
void hienThiGioHang(const DanhSach &ds);
