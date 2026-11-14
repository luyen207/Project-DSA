#pragma once

#include <string>
#include <vector>

#include "DoublyLinkedList.h"

// Tim kiem san pham trong gio hang.Chi doc, khong sua danh sach
namespace ProductSearch {

// Cat khoang trang 2 dau, doi chu A-Z thanh a-z 
std::string chuanHoaTen(const std::string& chuoi);

// Tim theo ID. nullptr = khong tim thay
// Con tro tro thang vao node trong danh sach: khong giu qua thao tac them/sua/xoa
const Node* timTheoId(const DanhSach& ds, int id);

// Tim theo ten: khong phan biet hoa thuong, khop mot phan
// Tra ve cac con tro node theo thu tu xuoi; vector rong = khong tim thay
// Cung khong giu qua thao tac them/sua/xoa
std::vector<const Node*> timTheoTen(const DanhSach& ds, const std::string& tuKhoa);

}  
