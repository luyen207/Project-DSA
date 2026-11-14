#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "DoublyLinkedList.h"

// Hien thi san pham dang bang. Chi doc, khong sua danh sach
// Moi ham nhan std::ostream& de co the kiem thu bang ostringstream
namespace ProductDisplay {

// Hien thi toan bo gio hang. xuoi = true: head -> tail (next);
// xuoi = false: tail -> head (prev). Gio rong thi in thong bao
void hienThiTatCa(const DanhSach& ds, bool xuoi = true, std::ostream& out = std::cout);

// Hien thi ket qua tim ten (ProductSearch::timTheoTen)
void hienThiDanhSach(const std::vector<const Node*>& danhSach, std::ostream& out = std::cout);

// Hien thi ket qua tim ID. node == nullptr: in thong bao khong tim thay
void hienThiMotSanPham(const Node* node, int id, std::ostream& out = std::cout);

// "Khong tim thay san pham co ten chua '<tuKhoa>'."
void hienThiKhongTimThayTen(const std::string& tuKhoa, std::ostream& out = std::cout);

// 1250000 -> "1.250.000" (gia la double, lam tron den dong)
// NaN / vo cuc / |so| >= 9e18 -> "---"
std::string dinhDangTien(double soTien);

}  
