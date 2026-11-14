// Chuong trinh dong lenh (CLI) cho gio hang. Dung de demo tich hop 4 module.
// Doc du lieu an toan: nhap sai (chu, de trong, qua lon) thi bao loi, khong crash.
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>

#include "CartController.h"
#include "ProductDisplay.h"

using namespace std;

// Doc 1 dong; tra ve false neu het du lieu nhap (Ctrl+D / Ctrl+Z)
static bool docDong(const string& loiNhac, string& dong) {
    cout << loiNhac;
    return static_cast<bool>(getline(cin, dong));
}

// Doc so nguyen: ketQua chi duoc ghi khi nhap dung
static bool docSoNguyen(const string& loiNhac, int& ketQua, bool& hetDuLieu) {
    string dong;
    if (!docDong(loiNhac, dong)) {
        hetDuLieu = true;
        return false;
    }
    char* cuoi = nullptr;
    errno = 0;
    long giaTri = strtol(dong.c_str(), &cuoi, 10);
    bool hopLe = !dong.empty() && *cuoi == '\0' && errno == 0 && giaTri >= -2147483647L && giaTri <= 2147483647L;
    if (!hopLe) {
        cout << "Gia tri khong phai so nguyen hop le.\n";
        return false;
    }
    ketQua = static_cast<int>(giaTri);
    return true;
}

static bool docSoThuc(const string& loiNhac, double& ketQua, bool& hetDuLieu) {
    string dong;
    if (!docDong(loiNhac, dong)) {
        hetDuLieu = true;
        return false;
    }
    char* cuoi = nullptr;
    errno = 0;
    double giaTri = strtod(dong.c_str(), &cuoi);
    bool hopLe = !dong.empty() && *cuoi == '\0' && errno == 0;
    if (!hopLe) {
        cout << "Gia tri khong phai so hop le.\n";
        return false;
    }
    ketQua = giaTri;   // NaN / vo cuc van duoc validation cua TV2 chan
    return true;
}

static void inKetQua(MaKetQua kq) {
    cout << (kq == THANH_CONG || kq == DA_CONG_DON ? "[OK] " : "[LOI] ") << moTaKetQua(kq) << "\n";
}

static void inMenu() {
    cout << "\n===== GIO HANG SHOPPRO =====\n"
         << " 1. Them san pham          2. Xoa san pham\n"
         << " 3. Sua ten + gia          4. Sua so luong\n"
         << " 5. Tim theo ID            6. Tim theo ten\n"
         << " 7. Hien thi xuoi          8. Hien thi nguoc\n"
         << " 9. Sap xep               10. Tong ket gio hang\n"
         << "11. Lam trong gio          0. Thoat\n";
}

int main() {
    CartController gio;
    bool hetDuLieu = false;
    bool dangChay = true;

    while (dangChay && !hetDuLieu) {
        inMenu();
        int chon = -1;
        if (docSoNguyen("Chon: ", chon, hetDuLieu)) {
            if (chon == 0) {
                dangChay = false;
            } else if (chon == 1) {
                int id = 0, soLuong = 0;
                double gia = 0;
                string ten;
                if (docSoNguyen("ID: ", id, hetDuLieu) && docDong("Ten: ", ten) &&
                    docSoThuc("Gia: ", gia, hetDuLieu) && docSoNguyen("So luong: ", soLuong, hetDuLieu)) {
                    inKetQua(gio.themSanPham(id, ten, gia, soLuong));
                }
            } else if (chon == 2) {
                int id = 0;
                if (docSoNguyen("ID can xoa: ", id, hetDuLieu)) {
                    inKetQua(gio.xoaSanPham(id));
                }
            } else if (chon == 3) {
                int id = 0;
                double gia = 0;
                string ten;
                if (docSoNguyen("ID: ", id, hetDuLieu) && docDong("Ten moi: ", ten) &&
                    docSoThuc("Gia moi: ", gia, hetDuLieu)) {
                    inKetQua(gio.capNhatThongTin(id, ten, gia));
                }
            } else if (chon == 4) {
                int id = 0, soLuong = 0;
                if (docSoNguyen("ID: ", id, hetDuLieu) && docSoNguyen("So luong moi: ", soLuong, hetDuLieu)) {
                    inKetQua(gio.capNhatSoLuong(id, soLuong));
                }
            } else if (chon == 5) {
                int id = 0;
                if (docSoNguyen("ID can tim: ", id, hetDuLieu)) {
                    gio.hienThiKetQuaTimId(id, cout);
                }
            } else if (chon == 6) {
                string tuKhoa;
                if (docDong("Tu khoa: ", tuKhoa)) {
                    gio.hienThiKetQuaTimTen(tuKhoa, cout);
                } else {
                    hetDuLieu = true;
                }
            } else if (chon == 7) {
                gio.hienThiTatCa(true, cout);
            } else if (chon == 8) {
                gio.hienThiTatCa(false, cout);
            } else if (chon == 9) {
                int kieu = 0;
                if (docSoNguyen("1=Gia tang 2=Gia giam 3=Ten 4=ID: ", kieu, hetDuLieu)) {
                    if (kieu == 1) {
                        gio.sapXep(CartController::GIA_TANG);
                    } else if (kieu == 2) {
                        gio.sapXep(CartController::GIA_GIAM);
                    } else if (kieu == 3) {
                        gio.sapXep(CartController::THEO_TEN);
                    } else if (kieu == 4) {
                        gio.sapXep(CartController::THEO_ID);
                    } else {
                        cout << "Lua chon sap xep khong hop le.\n";
                    }
                    if (kieu >= 1 && kieu <= 4) {
                        gio.hienThiTatCa(true, cout);
                    }
                }
            } else if (chon == 10) {
                cout << "So loai san pham: " << gio.soLoaiSanPham() << "\n"
                     << "Tong so luong:    " << gio.tongSoLuong() << "\n"
                     << "Tong tien:        " << ProductDisplay::dinhDangTien(gio.tongTien()) << " dong\n";
            } else if (chon == 11) {
                gio.lamTrongGio();
                cout << "[OK] Da lam trong gio hang.\n";
            } else {
                cout << "Lua chon khong hop le.\n";
            }
        }
    }
    cout << "Tam biet!\n";
    return 0;
}
