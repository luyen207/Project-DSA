#include "Node.h"

Node* taoNode(int id, string ten, double gia, int soLuong) {
    // Kiểm tra dữ liệu vào
    if (gia < 0 || soLuong <= 0 || ten == "") {
        return nullptr;
    }

    Node* nodeMoi = new Node;
    if (nodeMoi == nullptr) {
        return nullptr;
    }

    nodeMoi->id = id;
    nodeMoi->ten = ten;
    nodeMoi->gia = gia;
    nodeMoi->soLuong = soLuong;
    nodeMoi->prev = nullptr;   //Luôn chọn nullptr
    nodeMoi->next = nullptr;

    return nodeMoi;
}