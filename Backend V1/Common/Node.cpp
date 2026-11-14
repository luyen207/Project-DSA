#include "Node.h"
#include <cmath>
#include <new>

// Ten hop le khi co it nhat mot ky tu khong phai khoang trang
static bool tenCoNoiDung(const std::string& ten) {
    bool coNoiDung = false;
    for (size_t i = 0; i < ten.size(); i++) {
        if (ten[i] != ' ' && ten[i] != '\t' && ten[i] != '\n' && ten[i] != '\r') {
            coNoiDung = true;
        }
    }
    return coNoiDung;
}

Node* taoNode(int id, const std::string& ten, double gia, int soLuong) {
    // Kiem tra du lieu vao (khop voi ProductValidation cua TV2)
    if (id <= 0 || !tenCoNoiDung(ten)) {
        return nullptr;
    }
    // std::isfinite loai ca NaN lan vo cuc
    if (!std::isfinite(gia) || gia < 0 || gia > GIA_TOI_DA) {
        return nullptr;
    }
    if (soLuong <= 0 || soLuong > SO_LUONG_TOI_DA) {
        return nullptr;
    }

    // new (std::nothrow): het bo nho thi tra ve nullptr thay vi nem ngoai le
    Node* nodeMoi = new (std::nothrow) Node;
    if (nodeMoi == nullptr) {
        return nullptr;
    }

    nodeMoi->id = id;
    nodeMoi->ten = ten;
    nodeMoi->gia = gia;
    nodeMoi->soLuong = soLuong;
    nodeMoi->prev = nullptr;
    nodeMoi->next = nullptr;

    return nodeMoi;
}
