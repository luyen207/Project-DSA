#include <iostream>
#include "DoublyLinkedList.h"
using namespace std;

int main() {
    DanhSach ds;
    khoiTaoDanhSach(ds);
    cout << "Rong: " << laRong(ds) << endl;                 // 1
    cout << "Rong hop le: " << kiemTraRongHopLe(ds) << endl; // 1
    cout << "Lien ket: " << kiemTraLienKet(ds) << endl;      // 1

    // tao 3 node
    Node* a = taoNode(1, "Ao thun", 150000, 2);
    Node* b = taoNode(2, "Quan jean", 350000, 1);
    Node* c = taoNode(3, "Non luoi trai", 90000, 3);

    // test taoNode voi du lieu sai
    Node* sai = taoNode(4, "", -5, 0);
    cout << "Node sai la nullptr: " << (sai == nullptr) << endl; // 1

    // noi tay: a <-> b <-> c
    a->next = b;  b->prev = a;
    b->next = c;  c->prev = b;
    ds.head = a;
    ds.tail = c;
    ds.soNode = 3;

    cout << "Sau khi noi, lien ket: " << kiemTraLienKet(ds) << endl; // 1

    // pha lien ket co y: b->prev khong con tro ve a
    b->prev = nullptr;
    cout << "Sau khi pha, lien ket: " << kiemTraLienKet(ds) << endl; // 0
    b->prev = a;  // sua lai

    // pha soNode
    ds.soNode = 5;
    cout << "Sai soNode, lien ket: " << kiemTraLienKet(ds) << endl;  // 0
    ds.soNode = 3;

    giaiPhongDanhSach(ds);
    cout << "Sau giai phong, rong: " << laRong(ds) << endl;          // 1
    return 0;
}
