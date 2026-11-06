#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

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

#endif