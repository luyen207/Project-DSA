#pragma once

#include <string>

// ---- Gioi han du lieu (dung chung cho ca du an) ----
// Dat tran de tranh tran so khi cong don / nhan / cong tong
const int SO_LUONG_TOI_DA = 1000000;     // so luong toi da cua MOT dong trong gio
const double GIA_TOI_DA = 1000000000000.0; // gia toi da 1 san pham: 1.000 ty dong

// Mot dong trong gio hang
struct Node {
    int id;              // Ma san pham
    std::string ten;     // Ten san pham
    double gia;          // Gia san pham
    int soLuong;         // So luong
    Node* prev;          // Nut dung truoc
    Node* next;          // Nut dung sau
};

// Tao mot nut moi, tra ve nullptr neu du lieu khong hop le:
//   id > 0 | ten co it nhat 1 ky tu khong phai khoang trang
//   0 <= gia <= GIA_TOI_DA (khong NaN, khong vo cuc) | 1 <= soLuong <= SO_LUONG_TOI_DA
Node* taoNode(int id, const std::string& ten, double gia, int soLuong);
