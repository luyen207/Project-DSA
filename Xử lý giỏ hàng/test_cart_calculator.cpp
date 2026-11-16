#include <iostream>
#include <cmath>
#include "CartCalculator.h"
#include "ProductSort.h"

namespace {
int tong = 0, dat = 0;
void kiemTra(bool dieuKien, const char* ten) {
    ++tong;
    if (dieuKien) {
        ++dat;
    } else {
        std::cout << "[LOI] " << ten << "\n";
    }
}

// Ham tro giup: kiem tra danh sach da sap xep theo gia tang dan
bool kiemTraGiaTangDan(const DanhSach& ds) {
    if (laRong(ds) || ds.soNode <= 1) return true;
    const Node* p = ds.head;
    while (p != nullptr && p->next != nullptr) {
        if (p->gia > p->next->gia) return false;
        p = p->next;
    }
    return true;
}

// Ham tro giup: kiem tra danh sach da sap xep theo gia giam dan
bool kiemTraGiaGiamDan(const DanhSach& ds) {
    if (laRong(ds) || ds.soNode <= 1) return true;
    const Node* p = ds.head;
    while (p != nullptr && p->next != nullptr) {
        if (p->gia < p->next->gia) return false;
        p = p->next;
    }
    return true;
}

}  // namespace

int main() {
    std::cout << "=== BAT DAU KIEM THU MODULE CART PROCESSING (TV4) ===\n\n";

    // ========================
    // PHAN 1: CART CALCULATOR
    // ========================
    std::cout << "--- PHAN 1: CartCalculator ---\n";

    // TC_CALC_01: Gio rong
    DanhSach rong;
    khoiTaoDanhSach(rong);
    kiemTra(CartCalculator::tinhTongTien(rong) == 0, "TC_CALC_01 tong tien gio rong");

    // TC_CALC_02: 1 san pham
    DanhSach ds1;
    khoiTaoDanhSach(ds1);
    themCuoi(ds1, taoNode(1, "Chuot", 100000, 2));
    kiemTra(CartCalculator::tinhTongTien(ds1) == 200000, "TC_CALC_02 tong tien 1 sp");

    // TC_CALC_03: Nhieu san pham
    DanhSach ds;
    khoiTaoDanhSach(ds);
    themCuoi(ds, taoNode(1, "Chuot", 250000, 2));      // 500k
    themCuoi(ds, taoNode(2, "Ban phim", 890000, 1));   // 890k
    themCuoi(ds, taoNode(3, "Man hinh", 3200000, 1));  // 3200k
    // Tong = 500k + 890k + 3200k = 4590k
    kiemTra(CartCalculator::tinhTongTien(ds) == 4590000, "TC_CALC_03 tong tien nhieu sp");

    // TC_CALC_04: Gia = 0
    DanhSach ds2;
    khoiTaoDanhSach(ds2);
    themCuoi(ds2, taoNode(1, "Mien phi", 0, 5));
    kiemTra(CartCalculator::tinhTongTien(ds2) == 0, "TC_CALC_04 gia = 0");
    giaiPhongDanhSach(ds2);

    // TC_CALC_05: Gia lon
    DanhSach ds3;
    khoiTaoDanhSach(ds3);
    themCuoi(ds3, taoNode(1, "Sieu xe", 9999999999.0, 1));
    kiemTra(CartCalculator::tinhTongTien(ds3) == 9999999999.0, "TC_CALC_05 gia lon");
    giaiPhongDanhSach(ds3);

    // TC_CALC_06: Tong so luong gio rong
    kiemTra(CartCalculator::tinhTongSoLuong(rong) == 0, "TC_CALC_06 tong sl gio rong");

    // TC_CALC_07: Tong so luong nhieu sp
    kiemTra(CartCalculator::tinhTongSoLuong(ds) == 4, "TC_CALC_07 tong sl = 2+1+1");

    // TC_CALC_08: Dem so loai san pham
    kiemTra(CartCalculator::demSoLoaiSanPham(ds) == 3, "TC_CALC_08 dem 3 loai");

    // TC_CALC_09: Dem so loai gio rong
    kiemTra(CartCalculator::demSoLoaiSanPham(rong) == 0, "TC_CALC_09 dem gio rong");

    // TC_CALC_10: Gia trung binh
    // ds: (250k*2 + 890k*1 + 3200k*1) / 4 = 4590k / 4 = 1147500
    double giaTB = CartCalculator::tinhGiaTrungBinh(ds);
    kiemTra(std::abs(giaTB - 1147500.0) < 1.0, "TC_CALC_10 gia TB");

    // TC_CALC_11: Gia TB gio rong
    kiemTra(CartCalculator::tinhGiaTrungBinh(rong) == 0, "TC_CALC_11 gia TB gio rong");

    giaiPhongDanhSach(ds1);

    // ========================
    // PHAN 2: PRODUCT SORT
    // ========================
    std::cout << "\n--- PHAN 2: ProductSort ---\n";

    // TC_SORT_01: Sort gio rong
    ProductSort::sapXepTheoGiaTangDan(rong);
    kiemTra(laRong(rong), "TC_SORT_01 sort gio rong khong crash");

    // TC_SORT_02: Sort 1 node
    DanhSach ds4;
    khoiTaoDanhSach(ds4);
    themCuoi(ds4, taoNode(1, "Duy nhat", 100, 1));
    ProductSort::sapXepTheoGiaTangDan(ds4);
    kiemTra(ds4.head->id == 1, "TC_SORT_02 sort 1 node");
    giaiPhongDanhSach(ds4);

    // TC_SORT_03: Sort gia tang dan
    DanhSach ds5;
    khoiTaoDanhSach(ds5);
    themCuoi(ds5, taoNode(1, "C", 300000, 1));
    themCuoi(ds5, taoNode(2, "A", 100000, 1));
    themCuoi(ds5, taoNode(3, "B", 200000, 1));
    ProductSort::sapXepTheoGiaTangDan(ds5);
    kiemTra(kiemTraGiaTangDan(ds5) && ds5.head->gia == 100000, "TC_SORT_03 gia tang dan");

    // TC_SORT_04: Da sap xep san
    DanhSach ds6;
    khoiTaoDanhSach(ds6);
    themCuoi(ds6, taoNode(1, "A", 100, 1));
    themCuoi(ds6, taoNode(2, "B", 200, 1));
    themCuoi(ds6, taoNode(3, "C", 300, 1));
    ProductSort::sapXepTheoGiaTangDan(ds6);
    kiemTra(ds6.head->gia == 100 && ds6.tail->gia == 300, "TC_SORT_04 da sap san");
    giaiPhongDanhSach(ds6);

    // TC_SORT_05: Sort gia giam dan
    ProductSort::sapXepTheoGiaGiamDan(ds5);
    kiemTra(kiemTraGiaGiamDan(ds5) && ds5.head->gia == 300000, "TC_SORT_05 gia giam dan");
    giaiPhongDanhSach(ds5);

    // TC_SORT_06: Sort theo ten A-Z
    DanhSach ds7;
    khoiTaoDanhSach(ds7);
    themCuoi(ds7, taoNode(1, "Chuot", 100, 1));
    themCuoi(ds7, taoNode(2, "Ban phim", 200, 1));
    themCuoi(ds7, taoNode(3, "Ao", 300, 1));
    ProductSort::sapXepTheoTen(ds7);
    kiemTra(ds7.head->ten == "Ao", "TC_SORT_06 ten A-Z, 'Ao' dau tien");

    // TC_SORT_07: Ten khong phan biet hoa thuong
    DanhSach ds8;
    khoiTaoDanhSach(ds8);
    themCuoi(ds8, taoNode(1, "CHUOT", 100, 1));
    themCuoi(ds8, taoNode(2, "ban phim", 200, 1));
    ProductSort::sapXepTheoTen(ds8);
    kiemTra(ds8.head->ten == "ban phim", "TC_SORT_07 khong phan biet hoa thuong");
    giaiPhongDanhSach(ds8);

    // TC_SORT_08: Sort theo ID
    DanhSach ds9;
    khoiTaoDanhSach(ds9);
    themCuoi(ds9, taoNode(5, "E", 100, 1));
    themCuoi(ds9, taoNode(1, "A", 200, 1));
    themCuoi(ds9, taoNode(3, "C", 300, 1));
    ProductSort::sapXepTheoID(ds9);
    kiemTra(ds9.head->id == 1 && ds9.tail->id == 5, "TC_SORT_08 sort theo ID");

    // TC_SORT_09: Kiem tra lien ket sau sort
    kiemTra(kiemTraLienKet(ds9), "TC_SORT_09 lien ket hop le sau sort");
    giaiPhongDanhSach(ds9);

    // TC_SORT_10: Gia trung nhau
    DanhSach ds10;
    khoiTaoDanhSach(ds10);
    themCuoi(ds10, taoNode(1, "A", 100, 1));
    themCuoi(ds10, taoNode(2, "B", 100, 1));
    themCuoi(ds10, taoNode(3, "C", 100, 1));
    ProductSort::sapXepTheoGiaTangDan(ds10);
    kiemTra(kiemTraGiaTangDan(ds10), "TC_SORT_10 gia trung nhau");
    giaiPhongDanhSach(ds10);

    // TC_SORT_11: Danh sach lon hon
    DanhSach ds11;
    khoiTaoDanhSach(ds11);
    for (int i = 10; i >= 1; --i) {
        themCuoi(ds11, taoNode(i, "SP" + std::to_string(i), i * 100.0, 1));
    }
    ProductSort::sapXepTheoGiaTangDan(ds11);
    kiemTra(kiemTraGiaTangDan(ds11) && ds11.head->gia == 100, "TC_SORT_11 ds 10 node");
    giaiPhongDanhSach(ds11);

    // ========================
    // PHAN 3: TICH HOP
    // ========================
    std::cout << "\n--- PHAN 3: Test tich hop ---\n";

    // TC_INT_01: Tong tien khong doi sau sort
    double tongTienTruoc = CartCalculator::tinhTongTien(ds);
    ProductSort::sapXepTheoGiaTangDan(ds);
    double tongTienSau = CartCalculator::tinhTongTien(ds);
    kiemTra(tongTienTruoc == tongTienSau, "TC_INT_01 tong tien khong doi sau sort");

    // TC_INT_02: So loai khong doi sau sort
    int soLoaiTruoc = CartCalculator::demSoLoaiSanPham(ds7);
    ProductSort::sapXepTheoGiaGiamDan(ds7);
    int soLoaiSau = CartCalculator::demSoLoaiSanPham(ds7);
    kiemTra(soLoaiTruoc == soLoaiSau, "TC_INT_02 so loai khong doi");
    giaiPhongDanhSach(ds7);

    // TC_INT_03: Lien ket vong khong treo
    DanhSach vong;
    khoiTaoDanhSach(vong);
    themCuoi(vong, taoNode(1, "A", 100, 1));
    themCuoi(vong, taoNode(2, "B", 200, 1));
    vong.tail->next = vong.head;  // Tao vong

    // Tinh toan van chay duoc nho gioi han soNode
    double tongVong = CartCalculator::tinhTongTien(vong);
    kiemTra(tongVong == 300, "TC_INT_03 lien ket vong khong treo");

    vong.tail->next = nullptr;  // Sua lai
    giaiPhongDanhSach(vong);
    giaiPhongDanhSach(ds);

    // ========================
    // KET THUC
    // ========================
    std::cout << "\n=== KET QUA KIEM THU ===\n";
    std::cout << "Dat " << dat << " / " << tong << " test\n";

    if (dat == tong) {
        std::cout << "THANH CONG: Tat ca test deu pass!\n";
    } else {
        std::cout << "CO LOI: " << (tong - dat) << " test that bai\n";
    }

    return dat == tong ? 0 : 1;
}
