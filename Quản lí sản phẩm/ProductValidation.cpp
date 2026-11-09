#include "ProductValidation.h"
#include <cfloat>

const char* moTaKetQua(MaKetQua kq) {
    switch (kq) {
        case THANH_CONG:           return "Thanh cong";
        case DA_CONG_DON:          return "San pham da co trong gio, da cong don so luong";
        case ID_KHONG_HOP_LE:      return "ID khong hop le (phai > 0)";
        case TEN_KHONG_HOP_LE:     return "Ten san pham khong hop le (khong duoc rong)";
        case GIA_KHONG_HOP_LE:     return "Gia khong hop le (phai >= 0)";
        case SO_LUONG_KHONG_HOP_LE:return "So luong khong hop le (phai > 0)";
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
bool laTenHopLe(const string &ten) {
    for (size_t i = 0; i < ten.size(); i++) {
        if (ten[i] != ' ' && ten[i] != '\t' && ten[i] != '\n' && ten[i] != '\r') {
            return true;
        }
    }
    return false;
}

// gia == gia loai NaN; so sanh voi DBL_MAX loai vo cuc
bool laGiaHopLe(double gia) {
    return gia == gia && gia >= 0 && gia <= DBL_MAX;
}

bool laSoLuongHopLe(int soLuong) {
    return soLuong > 0;
}

bool daTonTaiID(const DanhSach &ds, int id) {
    return timNodeTheoID(ds, id) != nullptr;
}

MaKetQua kiemTraDuLieuSanPham(int id, const string &ten, double gia, int soLuong) {
    if (!laIDHopLe(id))           return ID_KHONG_HOP_LE;
    if (!laTenHopLe(ten))         return TEN_KHONG_HOP_LE;
    if (!laGiaHopLe(gia))         return GIA_KHONG_HOP_LE;
    if (!laSoLuongHopLe(soLuong)) return SO_LUONG_KHONG_HOP_LE;
    return THANH_CONG;
}
