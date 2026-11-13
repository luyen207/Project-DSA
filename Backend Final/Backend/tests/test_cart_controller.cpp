#include <iostream>
#include <sstream>
#include "CartController.h"

namespace {
int tong = 0, dat = 0;
void kiemTra(bool dieuKien, const char* ten) {
    ++tong;
    if (dieuKien) ++dat; else std::cout << "[LOI] " << ten << "\n";
}
}

int main() {
    CartController gio;
    kiemTra(gio.giaRong() && gio.tongTien() == 0 && gio.tongSoLuong() == 0, "TC_CTL_01 gio moi rong");

    kiemTra(gio.themSanPham(3, "Non", 50000, 3) == THANH_CONG, "TC_CTL_02 them");
    kiemTra(gio.themSanPham(1, "Ao", 100000, 2) == THANH_CONG, "TC_CTL_03 them 2");
    kiemTra(gio.themSanPham(1, "Ao", 100000, 1) == DA_CONG_DON, "TC_CTL_04 trung id -> cong don");
    kiemTra(gio.themSanPham(0, "X", 1, 1) == ID_KHONG_HOP_LE, "TC_CTL_05 id sai");
    kiemTra(gio.soLoaiSanPham() == 2 && gio.tongSoLuong() == 6, "TC_CTL_06 dem");
    kiemTra(gio.tongTien() == 450000, "TC_CTL_07 tong tien = 50000*3 + 100000*3");

    kiemTra(gio.timTheoId(1) != nullptr && gio.timTheoId(99) == nullptr, "TC_CTL_08 tim id");
    kiemTra(gio.timTheoTen("AO").size() == 1, "TC_CTL_09 tim ten");

    gio.sapXep(CartController::GIA_TANG);
    kiemTra(gio.timTheoTen("non").size() == 1 && gio.lienKetHopLe(), "TC_CTL_10 sap xep giu lien ket");
    std::ostringstream xuoi, nguoc;
    gio.hienThiTatCa(true, xuoi);
    gio.hienThiTatCa(false, nguoc);
    kiemTra(xuoi.str().find("Non") < xuoi.str().find("Ao"), "TC_CTL_11 gia tang: Non (50k) truoc Ao (100k)");
    kiemTra(nguoc.str().find("Ao") < nguoc.str().find("Non"), "TC_CTL_12 hien nguoc");

    kiemTra(gio.capNhatSoLuong(1, 10) == THANH_CONG && gio.capNhatThongTin(1, "Ao khoac", 1) == THANH_CONG, "TC_CTL_13 cap nhat");
    kiemTra(gio.xoaSanPham(99) == ID_KHONG_TON_TAI && gio.xoaSanPham(3) == THANH_CONG, "TC_CTL_14 xoa");
    kiemTra(gio.soLoaiSanPham() == 1 && gio.lienKetHopLe(), "TC_CTL_15 sau xoa");

    gio.lamTrongGio();
    kiemTra(gio.giaRong() && gio.lienKetHopLe(), "TC_CTL_16 lam trong");
    kiemTra(gio.themSanPham(7, "Moi", 1, 1) == THANH_CONG, "TC_CTL_17 dung lai sau khi lam trong");

    std::cout << "Dat " << dat << " / " << tong << " test\n";
    return dat == tong ? 0 : 1;
}
