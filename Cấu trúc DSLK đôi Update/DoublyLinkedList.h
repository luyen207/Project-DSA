#pragma once

#include "Node.h"

struct DanhSach {
    Node* head;     // Node đầu
    Node* tail;     // Node cuối
    int soNode;     // Số node hiện có
};

void khoiTaoDanhSach(DanhSach &ds);
bool laRong(const DanhSach &ds);
bool kiemTraRongHopLe(const DanhSach &ds);
void giaiPhongDanhSach(DanhSach &ds);
bool kiemTraLienKet(const DanhSach &ds);

// Thêm node vào đầu / cuối. Trả về false nếu nodeMoi là nullptr
bool themDau(DanhSach &ds, Node* nodeMoi);
bool themCuoi(DanhSach &ds, Node* nodeMoi);

// Chèn nodeMoi ngay sau node p (p phải nằm trong danh sách)
bool chenSau(DanhSach &ds, Node* p, Node* nodeMoi);

// Xóa node p (p phải nằm trong danh sách). Trả về false nếu không xóa được
bool xoaNode(DanhSach &ds, Node* p);

// Tìm node theo id. Không thấy thì trả về nullptr
Node* timNodeTheoID(const DanhSach &ds, int id);

// Xóa node theo id. Trả về false nếu không có id đó
bool xoaTheoID(DanhSach &ds, int id);

// Đổi chỗ DỮ LIỆU của 2 node (id, ten, gia, soLuong), KHÔNG đổi con trỏ
// Dùng cho phần sắp xếp của TV4
void hoanDoiDuLieu(Node* a, Node* b);