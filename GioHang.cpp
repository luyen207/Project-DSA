// ============================================================
//  CHƯƠNG TRÌNH GIỎ HÀNG ONLINE (bản console) - C++
//
//  Chức năng:
//    1. Xem danh sách sản phẩm
//    2. Thêm sản phẩm vào giỏ
//    3. Xem giỏ hàng
//    4. Đổi số lượng
//    5. Xóa sản phẩm khỏi giỏ
//    6. Thanh toán
//
// ============================================================

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>

#ifdef _WIN32
#include <windows.h>   // để hiển thị tiếng Việt trên Windows
#endif

using namespace std;

// ---------- 1. Cấu trúc dữ liệu ----------

// Một sản phẩm trong cửa hàng
struct SanPham {
    int id;
    string ten;
    double gia;     // đơn vị: VNĐ
    int tonKho;     // số lượng còn trong kho
};

// Một dòng trong giỏ hàng
struct MucGioHang {
    int idSanPham;
    string ten;
    double gia;
    int soLuong;
};

// ---------- 2. Hàm hỗ trợ nhập liệu ----------

// Nhập một số nguyên, nếu người dùng gõ chữ thì bắt nhập lại
int nhapSo(const string& loiNhac) {
    int x;
    while (true) {
        cout << loiNhac;
        if (cin >> x) {
            return x;
        }
        cout << "  [!] Vui long nhap mot so hop le.\n";
        cin.clear();                                          // xóa trạng thái lỗi
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // bỏ phần nhập sai
    }
}

// In tiền có dấu phân cách hàng nghìn: 1500000 -> 1,500,000
string dinhDangTien(double tien) {
    long long n = static_cast<long long>(tien);
    string s = to_string(n);
    string kq = "";
    int dem = 0;
    for (int i = static_cast<int>(s.size()) - 1; i >= 0; i--) {
        kq = s[i] + kq;
        dem++;
        if (dem % 3 == 0 && i != 0) kq = "," + kq;
    }
    return kq + " d";
}

// ---------- 3. Lớp quản lý cửa hàng và giỏ hàng ----------

class CuaHang {
private:
    vector<SanPham> danhSach;      // tất cả sản phẩm của cửa hàng
    vector<MucGioHang> gioHang;    // giỏ hàng của khách

    // Tìm sản phẩm theo id, trả về con trỏ (nullptr nếu không thấy)
    SanPham* timSanPham(int id) {
        for (size_t i = 0; i < danhSach.size(); i++) {
            if (danhSach[i].id == id) return &danhSach[i];
        }
        return nullptr;
    }

    // Tìm mục trong giỏ hàng theo id sản phẩm
    int timTrongGio(int id) {
        for (size_t i = 0; i < gioHang.size(); i++) {
            if (gioHang[i].idSanPham == id) return static_cast<int>(i);
        }
        return -1;
    }

public:
    // Tạo sẵn vài sản phẩm mẫu
    CuaHang() {
        danhSach.push_back({1, "Ao thun nam",     150000, 20});
        danhSach.push_back({2, "Quan jean nu",    350000, 15});
        danhSach.push_back({3, "Giay the thao",   800000, 10});
        danhSach.push_back({4, "Balo laptop",     450000, 8});
        danhSach.push_back({5, "Non luoi trai",    90000, 30});
    }

    // Hiển thị danh sách sản phẩm
    void hienThiSanPham() {
        cout << "\n=========== DANH SACH SAN PHAM ===========\n";
        cout << left << setw(5) << "ID" << setw(20) << "Ten"
             << setw(16) << "Gia" << "Kho\n";
        cout << "------------------------------------------\n";
        for (size_t i = 0; i < danhSach.size(); i++) {
            const SanPham& sp = danhSach[i];
            cout << left << setw(5) << sp.id << setw(20) << sp.ten
                 << setw(16) << dinhDangTien(sp.gia) << sp.tonKho << "\n";
        }
    }

    // Thêm sản phẩm vào giỏ
    void themVaoGio() {
        hienThiSanPham();
        int id = nhapSo("\nNhap ID san pham muon mua: ");
        SanPham* sp = timSanPham(id);
        if (sp == nullptr) {
            cout << "  [!] Khong tim thay san pham.\n";
            return;
        }

        int sl = nhapSo("Nhap so luong: ");
        if (sl <= 0) {
            cout << "  [!] So luong phai lon hon 0.\n";
            return;
        }

        int viTri = timTrongGio(id);
        int daCo = (viTri >= 0) ? gioHang[viTri].soLuong : 0;

        if (daCo + sl > sp->tonKho) {
            cout << "  [!] Khong du hang. Kho chi con " << sp->tonKho
                 << " (trong gio da co " << daCo << ").\n";
            return;
        }

        if (viTri >= 0) {
            gioHang[viTri].soLuong += sl;      // đã có trong giỏ -> cộng dồn
        } else {
            gioHang.push_back({sp->id, sp->ten, sp->gia, sl});
        }
        cout << "  Da them " << sl << " x " << sp->ten << " vao gio hang.\n";
    }

    // Xem giỏ hàng và tổng tiền
    void xemGioHang() {
        if (gioHang.empty()) {
            cout << "\n  Gio hang dang trong.\n";
            return;
        }
        cout << "\n============== GIO HANG CUA BAN ==============\n";
        cout << left << setw(5) << "ID" << setw(20) << "Ten"
             << setw(14) << "Don gia" << setw(5) << "SL" << "Thanh tien\n";
        cout << "----------------------------------------------\n";

        double tong = 0;
        for (size_t i = 0; i < gioHang.size(); i++) {
            const MucGioHang& m = gioHang[i];
            double thanhTien = m.gia * m.soLuong;
            tong += thanhTien;
            cout << left << setw(5) << m.idSanPham << setw(20) << m.ten
                 << setw(14) << dinhDangTien(m.gia) << setw(5) << m.soLuong
                 << dinhDangTien(thanhTien) << "\n";
        }
        cout << "----------------------------------------------\n";
        cout << "TONG CONG: " << dinhDangTien(tong) << "\n";
    }

    // Đổi số lượng của một sản phẩm trong giỏ
    void doiSoLuong() {
        if (gioHang.empty()) {
            cout << "\n  Gio hang dang trong.\n";
            return;
        }
        xemGioHang();
        int id = nhapSo("\nNhap ID san pham can doi so luong: ");
        int viTri = timTrongGio(id);
        if (viTri < 0) {
            cout << "  [!] San pham nay khong co trong gio.\n";
            return;
        }

        int slMoi = nhapSo("Nhap so luong moi (0 = xoa khoi gio): ");
        if (slMoi < 0) {
            cout << "  [!] So luong khong hop le.\n";
            return;
        }
        if (slMoi == 0) {
            gioHang.erase(gioHang.begin() + viTri);
            cout << "  Da xoa san pham khoi gio.\n";
            return;
        }

        SanPham* sp = timSanPham(id);
        if (slMoi > sp->tonKho) {
            cout << "  [!] Kho chi con " << sp->tonKho << " san pham.\n";
            return;
        }
        gioHang[viTri].soLuong = slMoi;
        cout << "  Da cap nhat so luong.\n";
    }

    // Xóa một sản phẩm khỏi giỏ
    void xoaKhoiGio() {
        if (gioHang.empty()) {
            cout << "\n  Gio hang dang trong.\n";
            return;
        }
        xemGioHang();
        int id = nhapSo("\nNhap ID san pham can xoa: ");
        int viTri = timTrongGio(id);
        if (viTri < 0) {
            cout << "  [!] San pham nay khong co trong gio.\n";
            return;
        }
        gioHang.erase(gioHang.begin() + viTri);
        cout << "  Da xoa san pham khoi gio.\n";
    }

    // Thanh toán: trừ kho và làm trống giỏ
    void thanhToan() {
        if (gioHang.empty()) {
            cout << "\n  Gio hang dang trong, khong the thanh toan.\n";
            return;
        }
        xemGioHang();

        cout << "\nXac nhan thanh toan? (1 = Dong y, 0 = Huy): ";
        int chon = nhapSo("");
        if (chon != 1) {
            cout << "  Da huy thanh toan.\n";
            return;
        }

        // Trừ số lượng trong kho
        for (size_t i = 0; i < gioHang.size(); i++) {
            SanPham* sp = timSanPham(gioHang[i].idSanPham);
            if (sp != nullptr) sp->tonKho -= gioHang[i].soLuong;
        }
        gioHang.clear();
        cout << "\n  *** THANH TOAN THANH CONG! Cam on ban da mua hang. ***\n";
    }
};

// ---------- 4. Hàm main: menu chính ----------

void hienThiMenu() {
    cout << "\n=============== MENU ===============\n";
    cout << " 1. Xem san pham\n";
    cout << " 2. Them san pham vao gio\n";
    cout << " 3. Xem gio hang\n";
    cout << " 4. Doi so luong\n";
    cout << " 5. Xoa san pham khoi gio\n";
    cout << " 6. Thanh toan\n";
    cout << " 0. Thoat\n";
    cout << "====================================\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    CuaHang shop;
    int luaChon;

    do {
        hienThiMenu();
        luaChon = nhapSo("Chon chuc nang: ");

        switch (luaChon) {
            case 1: shop.hienThiSanPham(); break;
            case 2: shop.themVaoGio();     break;
            case 3: shop.xemGioHang();     break;
            case 4: shop.doiSoLuong();     break;
            case 5: shop.xoaKhoiGio();     break;
            case 6: shop.thanhToan();      break;
            case 0: cout << "\nTam biet!\n"; break;
            default: cout << "  [!] Lua chon khong hop le.\n";
        }
    } while (luaChon != 0);

    return 0;
}