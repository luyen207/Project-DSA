#pragma once
#include <string>
#include "DoublyLinkedList.h"

// Ma ket qua dung chung cho ProductValidation va ProductManager
enum MaKetQua {
    THANH_CONG = 0,        // Thao tac thanh cong
    DA_CONG_DON,           // San pham da co trong gio -> da cong don so luong
    ID_KHONG_HOP_LE,       // id <= 0
    TEN_KHONG_HOP_LE,      // ten rong hoac chi toan khoang trang
    GIA_KHONG_HOP_LE,      // gia < 0, NaN, vo cuc hoac > GIA_TOI_DA
    SO_LUONG_KHONG_HOP_LE, // so luong <= 0 hoac > SO_LUONG_TOI_DA
    ID_KHONG_TON_TAI,      // khong tim thay id trong gio
    VUOT_SO_LUONG_TOI_DA,  // cong don lam so luong vuot qua SO_LUONG_TOI_DA
    LOI_BO_NHO             // khong tao duoc node moi
};

// Chuyen ma ket qua thanh cau thong bao tieng Viet khong dau
const char* moTaKetQua(MaKetQua kq);

// Kiem tra tung truong du lieu (true = hop le)
bool laIDHopLe(int id);
bool laTenHopLe(const std::string &ten);
bool laGiaHopLe(double gia);
bool laSoLuongHopLe(int soLuong);

// Kiem tra id da co trong danh sach chua (true = da trung)
bool daTonTaiID(const DanhSach &ds, int id);

// Kiem tra ca 4 truong cua mot san pham, tra ve loi dau tien gap phai
MaKetQua kiemTraDuLieuSanPham(int id, const std::string &ten, double gia, int soLuong);
