#include <iostream>
#include <sstream>

#include "ProductDisplay.h"
#include "ProductSearch.h"

namespace {
int tong = 0, dat = 0;
void kiemTra(bool dieuKien, const char* ten) {
    ++tong;
    if (dieuKien) ++dat; else std::cout << "[LOI] " << ten << "\n";
}
}  

int main() {
    DanhSach rong;
    khoiTaoDanhSach(rong);
    DanhSach ds;
    khoiTaoDanhSach(ds);
    themCuoi(ds, taoNode(1, "Chuot khong day", 250000, 2));
    themCuoi(ds, taoNode(2, "Ban phim co", 890000, 1));
    themCuoi(ds, taoNode(3, "Chuot gaming", 310000, 3));

    // --- tim theo ID ---
    const Node* a = ProductSearch::timTheoId(ds, 2);
    kiemTra(a != nullptr && a->ten == "Ban phim co", "TC_ID_01 tim thay");
    kiemTra(ProductSearch::timTheoId(ds, 999) == nullptr, "TC_ID_02 khong ton tai");
    kiemTra(ProductSearch::timTheoId(ds, -1) == nullptr, "TC_ID_03 id am");
    kiemTra(ProductSearch::timTheoId(rong, 1) == nullptr, "TC_ID_04 gio rong");

    // --- tim theo ten ---
    kiemTra(ProductSearch::timTheoTen(ds, "chuot").size() == 2, "TC_TEN_01 khop 2");
    kiemTra(ProductSearch::timTheoTen(ds, "CHUOT")[0]->id == 1, "TC_TEN_02 khong phan biet hoa thuong, thu tu xuoi");
    kiemTra(ProductSearch::timTheoTen(ds, "phim").size() == 1, "TC_TEN_03 khop mot phan");
    kiemTra(ProductSearch::timTheoTen(ds, "xyz").empty(), "TC_TEN_04 khong thay");
    kiemTra(ProductSearch::timTheoTen(ds, "   ").empty(), "TC_TEN_05 tu khoa rong");
    kiemTra(ProductSearch::timTheoTen(rong, "chuot").empty(), "TC_TEN_06 gio rong");

    // --- hien thi ---
    std::ostringstream o1;
    ProductDisplay::hienThiTatCa(rong, true, o1);
    kiemTra(o1.str().find("trong") != std::string::npos, "TC_HT_01 gio rong");

    std::ostringstream xuoi, nguoc;
    ProductDisplay::hienThiTatCa(ds, true, xuoi);
    ProductDisplay::hienThiTatCa(ds, false, nguoc);
    const std::string sx = xuoi.str(), sn = nguoc.str();
    kiemTra(sx.find("Chuot khong day") < sx.find("Chuot gaming"), "TC_HT_02 xuoi: id1 truoc id3");
    kiemTra(sn.find("Chuot gaming") < sn.find("Chuot khong day"), "TC_HT_03 nguoc: id3 truoc id1");
    kiemTra(sx.find("Tong so mat hang: 3") != std::string::npos, "TC_HT_04 dem 3");
    kiemTra(sx.find("500.000") != std::string::npos, "TC_HT_05 thanh tien 250000*2");

    std::ostringstream o2;
    ProductDisplay::hienThiMotSanPham(nullptr, 999, o2);
    kiemTra(o2.str().find("Khong tim thay") != std::string::npos, "TC_HT_06 khong thay");

    kiemTra(ProductDisplay::dinhDangTien(1250000) == "1.250.000", "TC_FMT_01");
    kiemTra(ProductDisplay::dinhDangTien(0) == "0", "TC_FMT_02");
    kiemTra(ProductDisplay::dinhDangTien(999) == "999", "TC_FMT_03");
    kiemTra(ProductDisplay::dinhDangTien(6300000000.0) == "6.300.000.000", "TC_FMT_04 qua 2,1 ty");
    kiemTra(ProductDisplay::dinhDangTien(1999.6) == "2.000", "TC_FMT_05 lam tron");

    // lien ket hong: tail->next tro lai head => khong treo
    DanhSach vong;
    khoiTaoDanhSach(vong);
    themCuoi(vong, taoNode(1, "A", 1, 1));
    themCuoi(vong, taoNode(2, "B", 1, 1));
    vong.tail->next = vong.head;
    std::ostringstream o3;
    ProductDisplay::hienThiTatCa(vong, true, o3);
    kiemTra(o3.str().find("Tong so mat hang: 2") != std::string::npos, "TC_HT_07 lien ket vong khong treo");
    vong.tail->next = nullptr;

    std::cout << "Dat " << dat << " / " << tong << " test\n";
    ProductDisplay::hienThiTatCa(ds, true);
    ProductDisplay::hienThiDanhSach(ProductSearch::timTheoTen(ds, "chuot"));

    giaiPhongDanhSach(ds);
    giaiPhongDanhSach(vong);
    return dat == tong ? 0 : 1;
}
