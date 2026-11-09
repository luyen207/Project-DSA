#include <iostream>
#include <climits>
#include <cmath>
#include "ProductManager.h"
using namespace std;

int tongTest = 0;
int soDat = 0;

void kiemTra(const string &tenTest, bool dieuKien) {
    tongTest++;
    if (dieuKien) { soDat++; cout << "[PASS] " << tenTest << endl; }
    else { cout << "[FAIL] " << tenTest << endl; }
}

// Danh sach phai lien ket dung va co dung n node
bool on(const DanhSach &ds, int n) {
    return kiemTraLienKet(ds) && ds.soNode == n;
}

int main() {
    DanhSach ds;
    khoiTaoDanhSach(ds);

    // ---------- Gio rong ----------
    kiemTra("Rong: xoa that bai", xoaSanPhamTheoID(ds, 1) == ID_KHONG_TON_TAI);
    kiemTra("Rong: cap nhat thong tin that bai", capNhatThongTin(ds, 1, "A", 1) == ID_KHONG_TON_TAI);
    kiemTra("Rong: cap nhat so luong that bai", capNhatSoLuong(ds, 1, 5) == ID_KHONG_TON_TAI);
    kiemTra("Rong: tong tien = 0", tinhTongTien(ds) == 0);
    kiemTra("Rong: daTonTaiID = false", !daTonTaiID(ds, 1));

    // ---------- Them: du lieu khong hop le ----------
    kiemTra("Them: id = 0", themSanPham(ds, 0, "Ao", 1000, 1) == ID_KHONG_HOP_LE);
    kiemTra("Them: id am", themSanPham(ds, -5, "Ao", 1000, 1) == ID_KHONG_HOP_LE);
    kiemTra("Them: ten rong", themSanPham(ds, 1, "", 1000, 1) == TEN_KHONG_HOP_LE);
    kiemTra("Them: ten toan khoang trang", themSanPham(ds, 1, "   ", 1000, 1) == TEN_KHONG_HOP_LE);
    kiemTra("Them: gia am", themSanPham(ds, 1, "Ao", -1, 1) == GIA_KHONG_HOP_LE);
    kiemTra("Them: gia NaN", themSanPham(ds, 1, "Ao", NAN, 1) == GIA_KHONG_HOP_LE);
    kiemTra("Them: gia vo cuc", themSanPham(ds, 1, "Ao", INFINITY, 1) == GIA_KHONG_HOP_LE);
    kiemTra("Them: so luong = 0", themSanPham(ds, 1, "Ao", 1000, 0) == SO_LUONG_KHONG_HOP_LE);
    kiemTra("Them: so luong am", themSanPham(ds, 1, "Ao", 1000, -3) == SO_LUONG_KHONG_HOP_LE);
    kiemTra("Them loi: gio van rong", laRong(ds) && kiemTraRongHopLe(ds));

    // ---------- Them hop le ----------
    kiemTra("Them san pham dau tien", themSanPham(ds, 1, "Ao", 100000, 2) == THANH_CONG);
    kiemTra("1 node: head == tail", ds.head == ds.tail && on(ds, 1));
    kiemTra("Them gia = 0 duoc phep", themSanPham(ds, 2, "Qua tang", 0, 1) == THANH_CONG);
    kiemTra("Them san pham thu 3", themSanPham(ds, 3, "Non", 50000, 3) == THANH_CONG);
    kiemTra("Them: dung thu tu (1,2,3)", ds.head->id == 1 && ds.tail->id == 3 && on(ds, 3));
    kiemTra("daTonTaiID: co", daTonTaiID(ds, 2));
    kiemTra("daTonTaiID: khong", !daTonTaiID(ds, 99));

    // ---------- Them trung ID ----------
    kiemTra("Trung id: cong don", themSanPham(ds, 1, "Ao", 100000, 5) == DA_CONG_DON);
    kiemTra("Trung id: so luong 2+5=7", timNodeTheoID(ds, 1)->soLuong == 7);
    kiemTra("Trung id: khong them node moi", on(ds, 3));
    kiemTra("Trung id khac ten/gia: giu ten gia cu",
            themSanPham(ds, 3, "Ten khac", 1, 1) == DA_CONG_DON &&
            timNodeTheoID(ds, 3)->ten == "Non" && timNodeTheoID(ds, 3)->gia == 50000 &&
            timNodeTheoID(ds, 3)->soLuong == 4);
    kiemTra("Trung id + du lieu sai: bao loi, khong doi",
            themSanPham(ds, 1, "Ao", 100000, 0) == SO_LUONG_KHONG_HOP_LE &&
            timNodeTheoID(ds, 1)->soLuong == 7);
    kiemTra("Cong don tran INT_MAX", themSanPham(ds, 2, "Qua tang", 0, INT_MAX - 1) == DA_CONG_DON &&
            themSanPham(ds, 2, "Qua tang", 0, 5) == VUOT_SO_LUONG_TOI_DA &&
            timNodeTheoID(ds, 2)->soLuong == INT_MAX);
    capNhatSoLuong(ds, 2, 1);   // tra ve gia tri nho cho cac test sau

    // ---------- Cap nhat thong tin ----------
    kiemTra("Cap nhat thong tin hop le", capNhatThongTin(ds, 1, "Ao khoac", 150000) == THANH_CONG);
    kiemTra("Cap nhat: ten, gia doi, so luong giu nguyen",
            timNodeTheoID(ds, 1)->ten == "Ao khoac" && timNodeTheoID(ds, 1)->gia == 150000 &&
            timNodeTheoID(ds, 1)->soLuong == 7);
    kiemTra("Cap nhat: id khong ton tai", capNhatThongTin(ds, 99, "X", 1) == ID_KHONG_TON_TAI);
    kiemTra("Cap nhat: id khong hop le", capNhatThongTin(ds, 0, "X", 1) == ID_KHONG_HOP_LE);
    kiemTra("Cap nhat: ten rong -> giu du lieu cu",
            capNhatThongTin(ds, 1, "", 1) == TEN_KHONG_HOP_LE && timNodeTheoID(ds, 1)->ten == "Ao khoac");
    kiemTra("Cap nhat: gia am -> giu du lieu cu",
            capNhatThongTin(ds, 1, "Y", -5) == GIA_KHONG_HOP_LE && timNodeTheoID(ds, 1)->gia == 150000);
    kiemTra("Cap nhat khong lam hong lien ket", on(ds, 3));

    // ---------- Cap nhat so luong ----------
    kiemTra("Cap nhat so luong hop le", capNhatSoLuong(ds, 3, 10) == THANH_CONG && timNodeTheoID(ds, 3)->soLuong == 10);
    kiemTra("Cap nhat so luong = 0 bi tu choi",
            capNhatSoLuong(ds, 3, 0) == SO_LUONG_KHONG_HOP_LE && timNodeTheoID(ds, 3)->soLuong == 10);
    kiemTra("Cap nhat so luong am bi tu choi", capNhatSoLuong(ds, 3, -1) == SO_LUONG_KHONG_HOP_LE);
    kiemTra("Cap nhat so luong: id khong ton tai", capNhatSoLuong(ds, 99, 5) == ID_KHONG_TON_TAI);
    kiemTra("Cap nhat so luong: id khong hop le", capNhatSoLuong(ds, -1, 5) == ID_KHONG_HOP_LE);

    // ---------- Tong tien ----------
    // 1: 150000*7 = 1050000 ; 2: 0*1 ; 3: 50000*10 = 500000
    kiemTra("Tong tien = 1550000", fabs(tinhTongTien(ds) - 1550000) < 1e-6);

    // ---------- Xoa ----------
    themSanPham(ds, 4, "Giay", 500000, 1);
    // Thu tu: 1 -> 2 -> 3 -> 4
    kiemTra("Xoa id khong hop le", xoaSanPhamTheoID(ds, 0) == ID_KHONG_HOP_LE);
    kiemTra("Xoa id khong ton tai", xoaSanPhamTheoID(ds, 99) == ID_KHONG_TON_TAI && on(ds, 4));
    kiemTra("Xoa node dau", xoaSanPhamTheoID(ds, 1) == THANH_CONG && ds.head->id == 2 && on(ds, 3));
    kiemTra("Xoa node cuoi", xoaSanPhamTheoID(ds, 4) == THANH_CONG && ds.tail->id == 3 && on(ds, 2));
    themSanPham(ds, 5, "Tui", 80000, 1);
    // Thu tu: 2 -> 3 -> 5
    kiemTra("Xoa node giua", xoaSanPhamTheoID(ds, 3) == THANH_CONG &&
            ds.head->next->id == 5 && on(ds, 2));
    kiemTra("Xoa 2 lan cung id: lan 2 that bai", xoaSanPhamTheoID(ds, 3) == ID_KHONG_TON_TAI);
    xoaSanPhamTheoID(ds, 2);
    kiemTra("Xoa node duy nhat", xoaSanPhamTheoID(ds, 5) == THANH_CONG && laRong(ds) && kiemTraRongHopLe(ds));
    kiemTra("Sau xoa het: them lai binh thuong", themSanPham(ds, 1, "Ao", 1, 1) == THANH_CONG && on(ds, 1));

    // ---------- Hien thi (kiem tra bang mat) ----------
    themSanPham(ds, 2, "Quan jean", 250000, 2);
    cout << endl;
    hienThiGioHang(ds);
    cout << endl;

    giaiPhongDanhSach(ds);
    kiemTra("Giai phong: ve rong", laRong(ds) && kiemTraRongHopLe(ds));

    cout << endl << "KET QUA: " << soDat << "/" << tongTest << " test dat" << endl;
    return soDat == tongTest ? 0 : 1;
}
