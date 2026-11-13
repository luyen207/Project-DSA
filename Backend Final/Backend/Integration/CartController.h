#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "DoublyLinkedList.h"
#include "ProductValidation.h"

// Lop dieu khien gio hang: MOT cua duy nhat de main (CLI) hoac REST API sau nay goi vao.
// - Moi gio hang so huu 1 DanhSach; huy CartController la tu giai phong
// - Tat ca thao tac thay doi gio deu tra ve MaKetQua (dung moTaKetQua de lay thong bao)
// - Khong copy duoc (DanhSach cam copy)
class CartController {
public:
    enum KieuSapXep { GIA_TANG, GIA_GIAM, THEO_TEN, THEO_ID };

    // ---- Quan ly san pham ----
    MaKetQua themSanPham(int id, const std::string& ten, double gia, int soLuong);
    MaKetQua xoaSanPham(int id);
    MaKetQua capNhatThongTin(int id, const std::string& ten, double gia);
    MaKetQua capNhatSoLuong(int id, int soLuong);
    void lamTrongGio();

    // ---- Tim kiem (con tro chi dung ngay, khong giu qua thao tac them/sua/xoa/sap xep) ----
    const Node* timTheoId(int id) const;
    std::vector<const Node*> timTheoTen(const std::string& tuKhoa) const;

    // ---- Hien thi ----
    void hienThiTatCa(bool xuoi, std::ostream& out) const;
    void hienThiKetQuaTimId(int id, std::ostream& out) const;
    void hienThiKetQuaTimTen(const std::string& tuKhoa, std::ostream& out) const;

    // ---- Xu ly ----
    void sapXep(KieuSapXep kieu);
    double tongTien() const;
    long long tongSoLuong() const;
    int soLoaiSanPham() const;
    bool giaRong() const;
    bool lienKetHopLe() const;   // dung cho kiem thu / chan doan

private:
    DanhSach ds;
};
