#include "ProductSort.h"
#include <string>

namespace ProductSort {

std::string chuanHoaTenDeSort(const std::string& ten) {
    std::string ketQua = ten;
    for (char& c : ketQua) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return ketQua;
}

void sapXepTheoGiaTangDan(DanhSach& ds) {
    if (laRong(ds) || ds.soNode <= 1) {
        return;
    }

    // Bubble sort: duyet nhieu lan, moi lan day phan tu lon nhat ve cuoi
    for (int i = 0; i < ds.soNode - 1; ++i) {
        Node* p = ds.head;
        int soBuoc = 0;

        // Moi lan duyet, so sanh va doi cho cac cap lien tiep
        for (int j = 0; j < ds.soNode - i - 1 && p != nullptr && soBuoc < ds.soNode; ++j, ++soBuoc) {
            Node* tiepTheo = p->next;
            if (tiepTheo == nullptr) break;

            // Neu gia node truoc > gia node sau thi doi cho
            if (p->gia > tiepTheo->gia) {
                hoanDoiDuLieu(p, tiepTheo);
            }

            p = p->next;
        }
    }
}

void sapXepTheoGiaGiamDan(DanhSach& ds) {
    if (laRong(ds) || ds.soNode <= 1) {
        return;
    }

    // Bubble sort, nhung doi chieu so sanh
    for (int i = 0; i < ds.soNode - 1; ++i) {
        Node* p = ds.head;
        int soBuoc = 0;

        for (int j = 0; j < ds.soNode - i - 1 && p != nullptr && soBuoc < ds.soNode; ++j, ++soBuoc) {
            Node* tiepTheo = p->next;
            if (tiepTheo == nullptr) break;

            // Neu gia node truoc < gia node sau thi doi cho (nguoc lai tang dan)
            if (p->gia < tiepTheo->gia) {
                hoanDoiDuLieu(p, tiepTheo);
            }

            p = p->next;
        }
    }
}

void sapXepTheoTen(DanhSach& ds) {
    if (laRong(ds) || ds.soNode <= 1) {
        return;
    }

    // Bubble sort theo ten (khong phan biet hoa thuong)
    for (int i = 0; i < ds.soNode - 1; ++i) {
        Node* p = ds.head;
        int soBuoc = 0;

        for (int j = 0; j < ds.soNode - i - 1 && p != nullptr && soBuoc < ds.soNode; ++j, ++soBuoc) {
            Node* tiepTheo = p->next;
            if (tiepTheo == nullptr) break;

            // So sanh ten (da chuan hoa chu thuong)
            std::string ten1 = chuanHoaTenDeSort(p->ten);
            std::string ten2 = chuanHoaTenDeSort(tiepTheo->ten);

            if (ten1 > ten2) {
                hoanDoiDuLieu(p, tiepTheo);
            }

            p = p->next;
        }
    }
}

void sapXepTheoID(DanhSach& ds) {
    if (laRong(ds) || ds.soNode <= 1) {
        return;
    }

    // Bubble sort theo ID
    for (int i = 0; i < ds.soNode - 1; ++i) {
        Node* p = ds.head;
        int soBuoc = 0;

        for (int j = 0; j < ds.soNode - i - 1 && p != nullptr && soBuoc < ds.soNode; ++j, ++soBuoc) {
            Node* tiepTheo = p->next;
            if (tiepTheo == nullptr) break;

            if (p->id > tiepTheo->id) {
                hoanDoiDuLieu(p, tiepTheo);
            }

            p = p->next;
        }
    }
}

}
