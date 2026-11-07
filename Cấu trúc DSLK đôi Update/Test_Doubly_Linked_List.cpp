#include <iostream>
#include "DoublyLinkedList.h"
using namespace std;

int tongTest = 0;
int soDat = 0;

void kiemTra(const string &tenTest, bool dieuKien) {
    tongTest++;
    if (dieuKien) {
        soDat++;
        cout << "[PASS] " << tenTest << endl;
    } else {
        cout << "[FAIL] " << tenTest << endl;
    }
}

// Duyệt ngược từ tail, đếm số node (để kiểm tra prev đúng)
int demNguoc(const DanhSach &ds) {
    int dem = 0;
    Node* p = ds.tail;
    while (p != nullptr) {
        dem++;
        p = p->prev;
    }
    return dem;
}

int main() {
    DanhSach ds;
    khoiTaoDanhSach(ds);

    // ---------- Danh sách rỗng ----------
    kiemTra("Rong: laRong", laRong(ds));
    kiemTra("Rong: lien ket hop le", kiemTraLienKet(ds));
    kiemTra("Rong: tim id tra ve nullptr", timNodeTheoID(ds, 1) == nullptr);
    kiemTra("Rong: xoaNode(nullptr) that bai", !xoaNode(ds, nullptr));
    kiemTra("Rong: xoaTheoID that bai", !xoaTheoID(ds, 1));

    // ---------- Thêm node ----------
    kiemTra("Them node khong hop le (nullptr)", !themCuoi(ds, nullptr));
    kiemTra("Them node dau tien", themCuoi(ds, taoNode(1, "Ao", 100000, 2)));
    kiemTra("1 node: head == tail", ds.head == ds.tail && ds.soNode == 1);
    kiemTra("1 node: lien ket hop le", kiemTraLienKet(ds));

    themCuoi(ds, taoNode(2, "Quan", 200000, 1));
    themCuoi(ds, taoNode(3, "Non", 50000, 3));
    themDau(ds, taoNode(4, "Giay", 500000, 1));
    // Thứ tự hiện tại: 4 -> 1 -> 2 -> 3
    kiemTra("Them nhieu node: soNode = 4", ds.soNode == 4);
    kiemTra("themDau: head la id 4", ds.head->id == 4);
    kiemTra("themCuoi: tail la id 3", ds.tail->id == 3);
    kiemTra("Nhieu node: lien ket hop le", kiemTraLienKet(ds));
    kiemTra("Duyet nguoc dem du 4 node", demNguoc(ds) == 4);

    // ---------- Tìm kiếm ----------
    Node* p = timNodeTheoID(ds, 2);
    kiemTra("Tim id ton tai", p != nullptr && p->ten == "Quan");
    kiemTra("Tim id khong ton tai", timNodeTheoID(ds, 99) == nullptr);

    // ---------- Chèn sau ----------
    kiemTra("chenSau node giua", chenSau(ds, p, taoNode(5, "Tui", 80000, 1)));
    // Thứ tự: 4 -> 1 -> 2 -> 5 -> 3
    kiemTra("chenSau: node 5 nam sau node 2", p->next->id == 5 && p->next->next->id == 3);
    kiemTra("chenSau: lien ket hop le", kiemTraLienKet(ds) && ds.soNode == 5);
    kiemTra("chenSau tail = themCuoi", chenSau(ds, ds.tail, taoNode(6, "Tat", 20000, 1)));
    // Thứ tự: 4 -> 1 -> 2 -> 5 -> 3 -> 6
    kiemTra("chenSau tail: tail doi thanh id 6", ds.tail->id == 6 && kiemTraLienKet(ds));

    // ---------- Hoán đổi dữ liệu ----------
    hoanDoiDuLieu(ds.head, ds.tail);
    // Thứ tự: 6 -> 1 -> 2 -> 5 -> 3 -> 4
    kiemTra("hoanDoiDuLieu: dau/cuoi doi cho", ds.head->id == 6 && ds.tail->id == 4);
    kiemTra("hoanDoiDuLieu: lien ket van hop le", kiemTraLienKet(ds));

    // ---------- Xóa node ----------
    kiemTra("Xoa node dau", xoaTheoID(ds, 6));
    kiemTra("Sau xoa dau: head la id 1", ds.head->id == 1 && ds.head->prev == nullptr);
    kiemTra("Sau xoa dau: lien ket hop le", kiemTraLienKet(ds));

    kiemTra("Xoa node cuoi", xoaTheoID(ds, 4));
    kiemTra("Sau xoa cuoi: tail la id 3", ds.tail->id == 3 && ds.tail->next == nullptr);
    kiemTra("Sau xoa cuoi: lien ket hop le", kiemTraLienKet(ds));

    kiemTra("Xoa node giua", xoaTheoID(ds, 2));
    // Thứ tự: 1 -> 5 -> 3
    kiemTra("Sau xoa giua: 1 noi voi 5", ds.head->next->id == 5 && ds.head->next->prev == ds.head);
    kiemTra("Sau xoa giua: lien ket hop le", kiemTraLienKet(ds) && ds.soNode == 3);
    kiemTra("Xoa id khong ton tai", !xoaTheoID(ds, 99));

    xoaTheoID(ds, 5);
    xoaTheoID(ds, 3);
    kiemTra("Xoa node duy nhat", xoaTheoID(ds, 1));
    kiemTra("Sau xoa het: danh sach rong hop le", laRong(ds) && kiemTraRongHopLe(ds) && ds.soNode == 0);
    kiemTra("Sau xoa het: lien ket hop le", kiemTraLienKet(ds));

    // ---------- Giải phóng ----------
    themCuoi(ds, taoNode(7, "A", 1, 1));
    themCuoi(ds, taoNode(8, "B", 2, 2));
    giaiPhongDanhSach(ds);
    kiemTra("Giai phong: ve rong", laRong(ds) && kiemTraRongHopLe(ds));

    cout << endl << "KET QUA: " << soDat << "/" << tongTest << " test dat" << endl;
    if (soDat == tongTest) {
        return 0;
    }
    return 1;
}