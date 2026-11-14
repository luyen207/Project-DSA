#include "ProductSearch.h"

namespace ProductSearch {

std::string chuanHoaTen(const std::string& chuoi) {
    const char* khoangTrang = " \t\r\n";
    const std::size_t dau = chuoi.find_first_not_of(khoangTrang);
    if (dau == std::string::npos) return "";
    const std::size_t cuoi = chuoi.find_last_not_of(khoangTrang);
    std::string ketQua = chuoi.substr(dau, cuoi - dau + 1);
    for (char& c : ketQua) {
        // chi doi chu A-Z
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return ketQua;
}

const Node* timTheoId(const DanhSach& ds, int id) {
    return timNodeTheoID(ds, id);  // ham cua TV1: khong thay thi tra nullptr
}

std::vector<const Node*> timTheoTen(const DanhSach& ds, const std::string& tuKhoa) {
    std::vector<const Node*> ketQua;
    const std::string khoa = chuanHoaTen(tuKhoa);
    if (khoa.empty()) return ketQua; 

    // duyet xuoi bang con tro next; gioi han soNode buoc phong truoc lien ket hong
    int soBuoc = 0;
    for (const Node* p = ds.head; p != nullptr && soBuoc < ds.soNode; p = p->next, ++soBuoc) {
        if (chuanHoaTen(p->ten).find(khoa) != std::string::npos) ketQua.push_back(p);
    }
    return ketQua;
}

}  
