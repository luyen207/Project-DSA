#include "ProductValidation.h"
#include <cmath>

const char* moTaKetQua(MaKetQua kq) {
    switch (kq) {
        case THANH_CONG:           return "Thanh cong";
        case DA_CONG_DON:          return "San pham da co trong gio, da cong don so luong";
        case ID_KHONG_HOP_LE:      return "ID khong hop le (phai > 0)";
        case TEN_KHONG_HOP_LE:     return "Ten san pham khong hop le (khong duoc rong)";
        case GIA_KHONG_HOP_LE:     return "Gia khong hop le (phai tu 0 den gia toi da)";
        case SO_LUONG_KHONG_HOP_LE:return "So luong khong hop le (phai tu 1 den so luong toi da)";
        case ID_KHONG_TON_TAI:     return "Khong tim thay ID trong gio hang";
        case VUOT_SO_LUONG_TOI_DA: return "So luong vuot qua gioi han cho phep";
        case LOI_BO_NHO:           return "Khong du bo nho de tao san pham";
    }
    return "Loi khong xac dinh";
}

bool laIDHopLe(int id) {
    return id > 0;
}

// Ten hop le khi co it nhat mot ky tu khong phai khoang trang
bool laTenHopLe(const std::string &ten) {
    for (size_t i = 0; i < ten.size(); i++) {
        if (ten[i] != ' ' && ten[i] != '\t' && ten[i] != '\n' && ten[i] != '\r') {
            return true;
        }
    }
    return false;
}

// isfinite loai NaN va vo cuc; them tran GIA_TOI_DA de tong tien khong tran
bool laGiaHopLe(double gia) {
    return std::isfinite(gia) && gia >= 0 && gia <= GIA_TOI_DA;
}

bool laSoLuongHopLe(int soLuong) {
    return soLuong > 0 && soLuong <= SO_LUONG_TOI_DA;
}

bool daTonTaiID(const DanhSach &ds, int id) {
    return timNodeTheoID(ds, id) != nullptr;
}

MaKetQua kiemTraDuLieuSanPham(int id, const std::string &ten, double gia, int soLuong) {
    if (!laIDHopLe(id))           return ID_KHONG_HOP_LE;
    if (!laTenHopLe(ten))         return TEN_KHONG_HOP_LE;
    if (!laGiaHopLe(gia))         return GIA_KHONG_HOP_LE;
    if (!laSoLuongHopLe(soLuong)) return SO_LUONG_KHONG_HOP_LE;
    return THANH_CONG;
}
