#pragma once

#include "DoublyLinkedList.h"

// Sap xep san pham trong gio hang theo cac tieu chi khac nhau
// SUA doi thu tu cac node trong danh sach bang cach doi cho DU LIEU (id, ten, gia, soLuong)
// KHONG doi con tro prev/next

namespace ProductSort {

// Sap xep theo gia TANG DAN (bubble sort on-place)
// Gio rong hoac chi co 1 node thi khong lam gi
void sapXepTheoGiaTangDan(DanhSach& ds);

// Sap xep theo gia GIAM DAN
void sapXepTheoGiaGiamDan(DanhSach& ds);

// Sap xep theo ten san pham (A-Z, khong phan biet hoa/thuong)
// Dung thuat toan bubble sort
void sapXepTheoTen(DanhSach& ds);

// Sap xep theo ID tang dan (id nho den lon)
void sapXepTheoID(DanhSach& ds);

// Ham tro giup: chuan hoa ten de so sanh (doi chu hoa thanh chu thuong)
// Dung noi bo, khong goi truc tiep
std::string chuanHoaTenDeSort(const std::string& ten);

}
