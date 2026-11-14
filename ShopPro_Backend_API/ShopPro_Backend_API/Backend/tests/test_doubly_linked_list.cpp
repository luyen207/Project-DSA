#include <iostream>
#include <cmath>
#include <climits>
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

    // ---------- Sau review: node la (khong thuoc danh sach) ----------
    {
        DanhSach a;
        themCuoi(a, taoNode(1, "A", 1, 1));
        themCuoi(a, taoNode(2, "B", 1, 1));
        Node* la = taoNode(9, "La", 1, 1);   // nam NGOAI danh sach a
        kiemTra("thuocDanhSach: node trong ds", thuocDanhSach(a, a.head));
        kiemTra("thuocDanhSach: node la", !thuocDanhSach(a, la));
        kiemTra("thuocDanhSach: nullptr", !thuocDanhSach(a, nullptr));
        kiemTra("xoaNode node la bi tu choi", !xoaNode(a, la));
        
        kiemTra("Sau tu choi: ds khong doi", a.soNode == 2 && kiemTraLienKet(a));
        delete la;
        // ds 1 node + node la: truoc day xoa nham va mat node that
        DanhSach b;
        themCuoi(b, taoNode(1, "A", 1, 1));
        Node* la2 = taoNode(9, "La", 1, 1);
        kiemTra("1 node + node la: xoaNode that bai", !xoaNode(b, la2) && b.soNode == 1 && b.head->id == 1);
        delete la2;
    }
    {
        // chenSau voi node la: node chen khong duoc vao ds, tu giai phong
        DanhSach c;
        themCuoi(c, taoNode(1, "A", 1, 1));
        themCuoi(c, taoNode(2, "B", 1, 1));
        Node* la = taoNode(9, "La", 1, 1);
        Node* x = taoNode(10, "X", 1, 1);
        kiemTra("chenSau p la tra ve false", !chenSau(c, la, x) && c.soNode == 2 && kiemTraLienKet(c));
        delete la;
        delete x;
    }

    // ---------- Sau review: taoNode kiem tra chat ----------
    kiemTra("taoNode: gia NaN bi tu choi", taoNode(1, "A", NAN, 1) == nullptr);
    kiemTra("taoNode: gia vo cuc bi tu choi", taoNode(1, "A", INFINITY, 1) == nullptr);
    kiemTra("taoNode: gia vuot GIA_TOI_DA", taoNode(1, "A", GIA_TOI_DA * 2, 1) == nullptr);
    kiemTra("taoNode: id = 0 bi tu choi", taoNode(0, "A", 1, 1) == nullptr);
    kiemTra("taoNode: id am bi tu choi", taoNode(-7, "A", 1, 1) == nullptr);
    kiemTra("taoNode: ten toan khoang trang", taoNode(1, "   ", 1, 1) == nullptr);
    kiemTra("taoNode: so luong vuot toi da", taoNode(1, "A", 1, SO_LUONG_TOI_DA + 1) == nullptr);
    {
        Node* bien1 = taoNode(1, "A", GIA_TOI_DA, SO_LUONG_TOI_DA);   // bien tren van hop le
        Node* bien2 = taoNode(1, "A", 0, 1);                           // bien duoi van hop le
        kiemTra("taoNode: gia tri bien hop le", bien1 != nullptr && bien2 != nullptr);
        delete bien1;
        delete bien2;
    }

    // ---------- Sau review: tu giai phong khi ra khoi pham vi (RAII) ----------
    {
        DanhSach tuHuy;
        themCuoi(tuHuy, taoNode(1, "A", 1, 1));
        themCuoi(tuHuy, taoNode(2, "B", 1, 1));
        // khong goi giaiPhongDanhSach: destructor lo. Neu sai, ASan se bao leak
    }
    {
        DanhSach moi;   // khong goi khoiTaoDanhSach van phai rong
        kiemTra("DanhSach moi tao: tu dong rong", laRong(moi) && kiemTraRongHopLe(moi));
    }

    cout << endl << "KET QUA: " << soDat << "/" << tongTest << " test dat" << endl;
    if (soDat == tongTest) {
        return 0;
    }
    return 1;
}
