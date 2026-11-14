#include "ProductDisplay.h"

#include <cmath>

namespace {

const int RONG_STT = 4;
const int RONG_ID = 6;
const int RONG_TEN = 26;
const int RONG_GIA = 14;
const int RONG_SL = 8;
const int RONG_TIEN = 16;

int doDaiHienThi(const std::string& s) {
    int dem = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) ++dem;
    }
    return dem;
}

std::string canhTrai(const std::string& s, int rong) {
    std::string kq;
    int dem = 0;
    for (char ch : s) {
        const unsigned char c = static_cast<unsigned char>(ch);
        if ((c & 0xC0) != 0x80) {
            if (dem == rong) break;  
            ++dem;
        }
        kq += ch;
    }
    return kq + std::string(static_cast<std::size_t>(rong - dem), ' ');
}

std::string canhPhai(const std::string& s, int rong) {
    const int doDai = doDaiHienThi(s);
    if (doDai >= rong) return canhTrai(s, rong);
    return std::string(static_cast<std::size_t>(rong - doDai), ' ') + s;
}

void inDuongKe(std::ostream& out) {
    const int tong = RONG_STT + RONG_ID + RONG_TEN + RONG_GIA + RONG_SL + RONG_TIEN + 5 * 2;
    out << std::string(static_cast<std::size_t>(tong), '-') << "\n";
}

void inTieuDe(std::ostream& out) {
    inDuongKe(out);
    out << canhPhai("STT", RONG_STT) << "  " << canhTrai("ID", RONG_ID) << "  "
        << canhTrai("Ten san pham", RONG_TEN) << "  " << canhPhai("Don gia", RONG_GIA)
        << "  " << canhPhai("SL", RONG_SL) << "  " << canhPhai("Thanh tien", RONG_TIEN)
        << "\n";
    inDuongKe(out);
}

void inDong(std::ostream& out, int stt, const Node& node) {
    const double thanhTien = node.gia * static_cast<double>(node.soLuong);
    out << canhPhai(std::to_string(stt), RONG_STT) << "  "
        << canhTrai(std::to_string(node.id), RONG_ID) << "  "
        << canhTrai(node.ten, RONG_TEN) << "  "
        << canhPhai(ProductDisplay::dinhDangTien(node.gia), RONG_GIA) << "  "
        << canhPhai(std::to_string(node.soLuong), RONG_SL) << "  "
        << canhPhai(ProductDisplay::dinhDangTien(thanhTien), RONG_TIEN) << "\n";
}

} 

namespace ProductDisplay {

std::string dinhDangTien(double soTien) {
    // NaN, vo cuc hoac qua lon: llround se cho ket qua sai -> in "---"
    if (!std::isfinite(soTien) || std::fabs(soTien) >= 9.0e18) {
        return "---";
    }
    const long long lamTron = std::llround(soTien);
    const bool am = lamTron < 0;
    // tranh tran khi lay tri tuyet doi cua gia tri nho nhat
    const unsigned long long giaTri =
        am ? 0ULL - static_cast<unsigned long long>(lamTron)
           : static_cast<unsigned long long>(lamTron);
    const std::string so = std::to_string(giaTri);
    std::string kq;
    int dem = 0;
    for (int i = static_cast<int>(so.size()) - 1; i >= 0; --i) {
        kq.insert(kq.begin(), so[static_cast<std::size_t>(i)]);
        if (++dem % 3 == 0 && i > 0) kq.insert(kq.begin(), '.');
    }
    return am ? "-" + kq : kq;
}

void hienThiTatCa(const DanhSach& ds, bool xuoi, std::ostream& out) {
    if (laRong(ds)) {
        out << "Gio hang dang trong.\n";
        return;
    }
    out << (xuoi ? "Danh sach san pham (thu tu xuoi: head -> tail)\n"
                 : "Danh sach san pham (thu tu nguoc: tail -> head)\n");
    inTieuDe(out);

    int stt = 1;
    const Node* p = xuoi ? ds.head : ds.tail;
    // stt <= soNode: neu lien ket hong (vong lap) van dung, khong treo chuong trinh
    while (p != nullptr && stt <= ds.soNode) {
        inDong(out, stt++, *p);
        p = xuoi ? p->next : p->prev;
    }
    inDuongKe(out);
    out << "Tong so mat hang: " << ds.soNode << "\n";
}

void hienThiDanhSach(const std::vector<const Node*>& danhSach, std::ostream& out) {
    if (danhSach.empty()) {
        out << "Khong co san pham nao de hien thi.\n";
        return;
    }
    inTieuDe(out);
    int stt = 1;
    for (const Node* p : danhSach) {
        if (p != nullptr) inDong(out, stt++, *p);
    }
    inDuongKe(out);
    out << "So ket qua: " << danhSach.size() << "\n";
}

void hienThiMotSanPham(const Node* node, int id, std::ostream& out) {
    if (node == nullptr) {
        out << "Khong tim thay san pham co ID " << id << ".\n";
        return;
    }
    inTieuDe(out);
    inDong(out, 1, *node);
    inDuongKe(out);
}

void hienThiKhongTimThayTen(const std::string& tuKhoa, std::ostream& out) {
    out << "Khong tim thay san pham co ten chua '" << tuKhoa << "'.\n";
}

}
