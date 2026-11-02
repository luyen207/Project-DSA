// ============================================================
//  GIỎ HÀNG ONLINE (bản console) - C++  |  Có TÌM KIẾM và SẮP XẾP
//  Dành cho người mới học lập trình
//
//  Chức năng:
//    1. Xem danh sách sản phẩm
//    2. Tìm kiếm theo tên (không phân biệt hoa/thường)
//    3. Lọc theo khoảng giá
//    4. Sắp xếp sản phẩm (theo giá, tên, tồn kho)
//    5. Thêm sản phẩm vào giỏ
//    6. Xem giỏ hàng
//    7. Đổi số lượng
//    8. Xóa sản phẩm khỏi giỏ
//    9. Thanh toán
//
//  Biên dịch:  g++ -std=c++11 gio_hang_tim_kiem_sap_xep.cpp -o gio_hang
//  Chạy:       ./gio_hang   (Windows: gio_hang.exe)
// ============================================================

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>   // std::sort
#include <cctype>      // std::tolower

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ---------- 1. Cấu trúc dữ liệu ----------

struct SanPham {
    int id;
    string ten;
    double gia;     // VNĐ
    int tonKho;
};

struct MucGioHang {
    int idSanPham;
    string ten;
    double gia;
    int soLuong;
};

// ---------- 2. Hàm hỗ trợ ----------

// Nhập một số nguyên; nhập sai thì bắt nhập lại
int nhapSo(const string& loiNhac) {
    int x;
    while (true) {
        cout << loiNhac;
        if (cin >> x) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // bỏ phần thừa trên dòng
            return x;
        }
        cout << "  [!] Vui long nhap mot so hop le.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Chuyển chuỗi thành chữ thường để tìm kiếm không phân biệt hoa/thường
string chuThuong(string s) {
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = static_cast<char>(tolower(static_cast<unsigned char>(s[i])));
    }
    return s;
}

// 1500000 -> "1,500,000 d"
string dinhDangTien(double tien) {
    string s = to_string(static_cast<long long>(tien));
    string kq = "";
    int dem = 0;
    for (int i = static_cast<int>(s.size()) - 1; i >= 0; i--) {
        kq = s[i] + kq;
        dem++;
        if (dem % 3 == 0 && i != 0) kq = "," + kq;
    }
    return kq + " d";
}

// In một bảng sản phẩm (dùng chung cho xem, tìm kiếm, lọc)
void inBangSanPham(const vector<SanPham>& ds) {
    if (ds.empty()) {
        cout << "\n  Khong co san pham nao phu hop.\n";
        return;
    }
    cout << "\n=============== DANH SACH SAN PHAM ===============\n";
    cout << left << setw(5) << "ID" << setw(22) << "Ten"
         << setw(16) << "Gia" << "Kho\n";
    cout << "--------------------------------------------------\n";
    for (size_t i = 0; i < ds.size(); i++) {
        cout << left << setw(5) << ds[i].id << setw(22) << ds[i].ten
             << setw(16) << dinhDangTien(ds[i].gia) << ds[i].tonKho << "\n";
    }
    cout << "(" << ds.size() << " san pham)\n";
}

// ---------- 3. Lớp cửa hàng ----------

class CuaHang {
private:
    vector<SanPham> danhSach;
    vector<MucGioHang> gioHang;

    SanPham* timSanPham(int id) {
        for (size_t i = 0; i < danhSach.size(); i++) {
            if (danhSach[i].id == id) return &danhSach[i];
        }
        return nullptr;
    }

    int timTrongGio(int id) {
        for (size_t i = 0; i < gioHang.size(); i++) {
            if (gioHang[i].idSanPham == id) return static_cast<int>(i);
        }
        return -1;
    }

public:
    CuaHang() {
        danhSach.push_back({1,  "Ao thun nam",        150000, 20});
        danhSach.push_back({2,  "Quan jean nu",       350000, 15});
        danhSach.push_back({3,  "Giay the thao",      800000, 10});
        danhSach.push_back({4,  "Balo laptop",        450000,  8});
        danhSach.push_back({5,  "Non luoi trai",       90000, 30});
        danhSach.push_back({6,  "Ao khoac gio",       520000, 12});
        danhSach.push_back({7,  "Dep sandal",         180000, 25});
        danhSach.push_back({8,  "Quan short nam",     130000, 18});
        danhSach.push_back({9,  "Tui xach nu",        670000,  6});
        danhSach.push_back({10, "Giay luoi nam",      390000, 14});
    }

    // ----- XEM / TÌM KIẾM / LỌC / SẮP XẾP -----

    void hienThiSanPham() {
        inBangSanPham(danhSach);
    }

    // Tìm theo từ khóa trong tên sản phẩm
    void timKiem() {
        cout << "\nNhap tu khoa (vd: ao, giay, quan): ";
        string tuKhoa;
        getline(cin, tuKhoa);
        tuKhoa = chuThuong(tuKhoa);

        vector<SanPham> ketQua;
        for (size_t i = 0; i < danhSach.size(); i++) {
            if (chuThuong(danhSach[i].ten).find(tuKhoa) != string::npos) {
                ketQua.push_back(danhSach[i]);
            }
        }
        inBangSanPham(ketQua);
    }

    // Lọc theo khoảng giá
    void locTheoGia() {
        int giaMin = nhapSo("\nGia thap nhat (d): ");
        int giaMax = nhapSo("Gia cao nhat (d): ");
        if (giaMin > giaMax) {
            swap(giaMin, giaMax);   // người dùng nhập ngược thì đổi chỗ
        }

        vector<SanPham> ketQua;
        for (size_t i = 0; i < danhSach.size(); i++) {
            if (danhSach[i].gia >= giaMin && danhSach[i].gia <= giaMax) {
                ketQua.push_back(danhSach[i]);
            }
        }
        inBangSanPham(ketQua);
    }

    // Sắp xếp danh sách sản phẩm bằng std::sort
    void sapXep() {
        cout << "\n--- SAP XEP THEO ---\n";
        cout << " 1. Gia tang dan\n";
        cout << " 2. Gia giam dan\n";
        cout << " 3. Ten A -> Z\n";
        cout << " 4. Ten Z -> A\n";
        cout << " 5. Ton kho nhieu -> it\n";
        cout << " 6. Ma san pham (mac dinh)\n";
        int chon = nhapSo("Chon: ");

        // Mỗi lần so sánh, hàm lambda cho biết a có đứng trước b hay không
        switch (chon) {
            case 1:
                sort(danhSach.begin(), danhSach.end(),
                     [](const SanPham& a, const SanPham& b) { return a.gia < b.gia; });
                break;
            case 2:
                sort(danhSach.begin(), danhSach.end(),
                     [](const SanPham& a, const SanPham& b) { return a.gia > b.gia; });
                break;
            case 3:
                sort(danhSach.begin(), danhSach.end(),
                     [](const SanPham& a, const SanPham& b) {
                         return chuThuong(a.ten) < chuThuong(b.ten);
                     });
                break;
            case 4:
                sort(danhSach.begin(), danhSach.end(),
                     [](const SanPham& a, const SanPham& b) {
                         return chuThuong(a.ten) > chuThuong(b.ten);
                     });
                break;
            case 5:
                sort(danhSach.begin(), danhSach.end(),
                     [](const SanPham& a, const SanPham& b) { return a.tonKho > b.tonKho; });
                break;
            case 6:
                sort(danhSach.begin(), danhSach.end(),
                     [](const SanPham& a, const SanPham& b) { return a.id < b.id; });
                break;
            default:
                cout << "  [!] Lua chon khong hop le.\n";
                return;
        }
        cout << "  Da sap xep xong!\n";
        inBangSanPham(danhSach);
    }

    // ----- GIỎ HÀNG -----

    void themVaoGio() {
        inBangSanPham(danhSach);
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
            gioHang[viTri].soLuong += sl;
        } else {
            gioHang.push_back({sp->id, sp->ten, sp->gia, sl});
        }
        cout << "  Da them " << sl << " x " << sp->ten << " vao gio hang.\n";
    }

    void xemGioHang() {
        if (gioHang.empty()) {
            cout << "\n  Gio hang dang trong.\n";
            return;
        }
        cout << "\n================ GIO HANG CUA BAN ================\n";
        cout << left << setw(5) << "ID" << setw(22) << "Ten"
             << setw(14) << "Don gia" << setw(5) << "SL" << "Thanh tien\n";
        cout << "--------------------------------------------------\n";

        double tong = 0;
        for (size_t i = 0; i < gioHang.size(); i++) {
            const MucGioHang& m = gioHang[i];
            double thanhTien = m.gia * m.soLuong;
            tong += thanhTien;
            cout << left << setw(5) << m.idSanPham << setw(22) << m.ten
                 << setw(14) << dinhDangTien(m.gia) << setw(5) << m.soLuong
                 << dinhDangTien(thanhTien) << "\n";
        }
        cout << "--------------------------------------------------\n";
        cout << "TONG CONG: " << dinhDangTien(tong) << "\n";
    }

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

    void thanhToan() {
        if (gioHang.empty()) {
            cout << "\n  Gio hang dang trong, khong the thanh toan.\n";
            return;
        }
        xemGioHang();

        int chon = nhapSo("\nXac nhan thanh toan? (1 = Dong y, 0 = Huy): ");
        if (chon != 1) {
            cout << "  Da huy thanh toan.\n";
            return;
        }

        for (size_t i = 0; i < gioHang.size(); i++) {
            SanPham* sp = timSanPham(gioHang[i].idSanPham);
            if (sp != nullptr) sp->tonKho -= gioHang[i].soLuong;
        }
        gioHang.clear();
        cout << "\n  *** THANH TOAN THANH CONG! Cam on ban da mua hang. ***\n";
    }
};

// ---------- 4. Menu chính ----------

void hienThiMenu() {
    cout << "\n================== MENU ==================\n";
    cout << " 1. Xem san pham\n";
    cout << " 2. Tim kiem san pham theo ten\n";
    cout << " 3. Loc san pham theo khoang gia\n";
    cout << " 4. Sap xep san pham\n";
    cout << " 5. Them san pham vao gio\n";
    cout << " 6. Xem gio hang\n";
    cout << " 7. Doi so luong\n";
    cout << " 8. Xoa san pham khoi gio\n";
    cout << " 9. Thanh toan\n";
    cout << " 0. Thoat\n";
    cout << "==========================================\n";
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
            case 2: shop.timKiem();        break;
            case 3: shop.locTheoGia();     break;
            case 4: shop.sapXep();         break;
            case 5: shop.themVaoGio();     break;
            case 6: shop.xemGioHang();     break;
            case 7: shop.doiSoLuong();     break;
            case 8: shop.xoaKhoiGio();     break;
            case 9: shop.thanhToan();      break;
            case 0: cout << "\nTam biet!\n"; break;
            default: cout << "  [!] Lua chon khong hop le.\n";
        }
    } while (luaChon != 0);

    return 0;
}