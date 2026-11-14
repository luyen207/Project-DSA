#pragma once

#include "Node.h"

struct DanhSach {
    Node* head;     // Node dau
    Node* tail;     // Node cuoi
    int soNode;     // So node hien co

    DanhSach();                                 // Tu dong ve trang thai rong
    ~DanhSach();                                // Tu dong giai phong moi node
    DanhSach(const DanhSach&) = delete;         // CAM sao chep (tranh giai phong 2 lan)
    DanhSach& operator=(const DanhSach&) = delete;
};

void khoiTaoDanhSach(DanhSach &ds);
bool laRong(const DanhSach &ds);
bool kiemTraRongHopLe(const DanhSach &ds);
void giaiPhongDanhSach(DanhSach &ds);
bool kiemTraLienKet(const DanhSach &ds);

// Thêm node vào đầu / cuối. Trả về false nếu nodeMoi là nullptr
bool themDau(DanhSach &ds, Node* nodeMoi);
bool themCuoi(DanhSach &ds, Node* nodeMoi);

// Kiem tra node p co nam trong danh sach hay khong (O(n))
bool thuocDanhSach(const DanhSach &ds, const Node* p);

// Chen nodeMoi ngay sau node p. Tra ve false neu p khong thuoc danh sach
bool chenSau(DanhSach &ds, Node* p, Node* nodeMoi);

// Xoa node p. Tra ve false neu p la nullptr hoac KHONG thuoc danh sach
bool xoaNode(DanhSach &ds, Node* p);

// Tìm node theo id. Không thấy thì trả về nullptr
Node* timNodeTheoID(const DanhSach &ds, int id);

// Xóa node theo id. Trả về false nếu không có id đó
bool xoaTheoID(DanhSach &ds, int id);

// Đổi chỗ DỮ LIỆU của 2 node (id, ten, gia, soLuong), KHÔNG đổi con trỏ
// Dùng cho phần sắp xếp của TV4
void hoanDoiDuLieu(Node* a, Node* b);
