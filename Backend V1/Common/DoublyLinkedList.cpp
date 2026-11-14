#include "DoublyLinkedList.h"

DanhSach::DanhSach() : head(nullptr), tail(nullptr), soNode(0) {}

// Huy danh sach la tu giai phong, khong con ro bo nho neu quen goi giaiPhongDanhSach
DanhSach::~DanhSach() {
    giaiPhongDanhSach(*this);
}

// Đưa danh sách về trạng thái rỗng
void khoiTaoDanhSach(DanhSach &ds) {
    ds.head = nullptr;
    ds.tail = nullptr;
    ds.soNode = 0;
}

// Danh sách rỗng khi không có nút đầu
bool laRong(const DanhSach &ds) {
    return ds.head == nullptr;
}

// Kiểm tra kỹ: cả 3 trường phải cùng nói "rỗng"
// Trạng thái rỗng thực sự: head = nullptr, tail = nullptr, soNode = 0
bool kiemTraRongHopLe(const DanhSach &ds) {
    bool headRong = (ds.head == nullptr);
    bool tailRong = (ds.tail == nullptr);
    bool demBang0 = (ds.soNode == 0);

    // Ba giá trị phải đồng nhất (cùng true hoặc cùng false)
    return (headRong == tailRong) && (tailRong == demBang0);
}

// Xóa từng nút, rồi đưa danh sách về trạng thái rỗng
void giaiPhongDanhSach(DanhSach &ds) {
    Node* hienTai = ds.head;
    while (hienTai != nullptr) {
        Node* nodeKeTiep = hienTai->next;  // Lưu trước khi giải phóng nút
        delete hienTai;
        hienTai = nodeKeTiep;
    }
    ds.head = nullptr;
    ds.tail = nullptr;
    ds.soNode = 0;
}
// Kiểm tra danh sách có liên kết đúng không
// Trả về true nếu hợp lệ, false nếu có lỗi
bool kiemTraLienKet(const DanhSach &ds) {
    // Trường hợp rỗng: phải đúng trạng thái rỗng chuẩn
    if (ds.head == nullptr && ds.tail == nullptr) {
        return ds.soNode == 0;
    }
    // head và tail phải cùng khác nullptr
    if (ds.head == nullptr || ds.tail == nullptr) {
        return false;
    }

    bool hopLe = true;

    // Nút đầu không được có nút trước, nút cuối không được có nút sau
    if (ds.head->prev != nullptr) {
        hopLe = false;
    }
    if (ds.tail->next != nullptr) {
        hopLe = false;
    }

    int dem = 0;
    Node* hienTai = ds.head;
    Node* nodeCuoi = nullptr;

    while (hienTai != nullptr && hopLe) {
        dem++;

        // Chống vòng lặp vô hạn (next quay lại nút cũ)
        if (dem > ds.soNode) {
            hopLe = false;
        }

        // Quy tắc vàng: nếu A->next = B thì B->prev phải bằng A
        if (hopLe && hienTai->next != nullptr) {
            if (hienTai->next->prev != hienTai) {
                hopLe = false;
            }
        }

        nodeCuoi = hienTai;
        hienTai = hienTai->next;
    }

    // Nút cuối duyệt được phải chính là tail
    if (hopLe && nodeCuoi != ds.tail) {
        hopLe = false;
    }
    // Số nút đếm được phải khớp với soNode
    if (hopLe && dem != ds.soNode) {
        hopLe = false;
    }

    return hopLe;
}

// =====================================================
// PHẦN NÂNG CẤP
// =====================================================

// Thêm vào đầu danh sách
bool themDau(DanhSach &ds, Node* nodeMoi) {
    if (nodeMoi == nullptr) {
        return false;
    }
    nodeMoi->prev = nullptr;

    if (laRong(ds)) {
        // Danh sách rỗng: node mới vừa là head vừa là tail
        nodeMoi->next = nullptr;
        ds.head = nodeMoi;
        ds.tail = nodeMoi;
    } else {
        nodeMoi->next = ds.head;   // Node mới trỏ sang head cũ
        ds.head->prev = nodeMoi;   // Head cũ trỏ ngược về node mới
        ds.head = nodeMoi;         // Cập nhật head
    }
    ds.soNode++;
    return true;
}

// Thêm vào cuối danh sách
bool themCuoi(DanhSach &ds, Node* nodeMoi) {
    if (nodeMoi == nullptr) {
        return false;
    }
    nodeMoi->next = nullptr;

    if (laRong(ds)) {
        nodeMoi->prev = nullptr;
        ds.head = nodeMoi;
        ds.tail = nodeMoi;
    } else {
        nodeMoi->prev = ds.tail;   // Node mới trỏ ngược về tail cũ
        ds.tail->next = nodeMoi;   // Tail cũ trỏ tới node mới
        ds.tail = nodeMoi;         // Cập nhật tail
    }
    ds.soNode++;
    return true;
}

// Kiem tra p co thuoc danh sach khong (duyet toi da soNode buoc)
bool thuocDanhSach(const DanhSach &ds, const Node* p) {
    if (p == nullptr) {
        return false;
    }
    bool timThay = false;
    int soBuoc = 0;
    const Node* hienTai = ds.head;
    while (hienTai != nullptr && soBuoc < ds.soNode && !timThay) {
        if (hienTai == p) {
            timThay = true;
        }
        hienTai = hienTai->next;
        soBuoc++;
    }
    return timThay;
}

// Chèn nodeMoi ngay sau node p
bool chenSau(DanhSach &ds, Node* p, Node* nodeMoi) {
    if (p == nullptr || nodeMoi == nullptr) {
        return false;
    }
    // p phai nam trong danh sach, neu khong se hong lien ket / crash
    if (!thuocDanhSach(ds, p)) {
        return false;
    }
    // p là node cuối thì chèn sau = thêm cuối
    if (p == ds.tail) {
        return themCuoi(ds, nodeMoi);
    }

    // Trường hợp p ở đầu hoặc giữa: p->next chắc chắn khác nullptr
    Node* nodeSau = p->next;

    nodeMoi->prev = p;
    nodeMoi->next = nodeSau;
    p->next = nodeMoi;
    nodeSau->prev = nodeMoi;

    ds.soNode++;
    return true;
}

// Xóa node p: có 4 trường hợp
bool xoaNode(DanhSach &ds, Node* p) {
    if (p == nullptr || laRong(ds)) {
        return false;
    }
    // p phai nam trong danh sach, neu khong se xoa nham node la va lam mat node that
    if (!thuocDanhSach(ds, p)) {
        return false;
    }

    if (ds.head == ds.tail) {
        // TH1: node duy nhất -> danh sách về rỗng
        ds.head = nullptr;
        ds.tail = nullptr;
    } else if (p == ds.head) {
        // TH2: xóa node đầu
        ds.head = p->next;
        ds.head->prev = nullptr;
    } else if (p == ds.tail) {
        // TH3: xóa node cuối
        ds.tail = p->prev;
        ds.tail->next = nullptr;
    } else {
        // TH4: xóa node giữa -> nối hai node kề bên lại với nhau
        p->prev->next = p->next;
        p->next->prev = p->prev;
    }

    delete p;
    ds.soNode--;
    return true;
}

// Tìm node theo id
Node* timNodeTheoID(const DanhSach &ds, int id) {
    Node* hienTai = ds.head;
    Node* ketQua = nullptr;
    bool daTim = false;

    int soBuoc = 0;   // gioi han soNode buoc: lien ket vong khong lam treo chuong trinh

    while (hienTai != nullptr && !daTim && soBuoc < ds.soNode) {
        if (hienTai->id == id) {
            ketQua = hienTai;
            daTim = true;
        }
        hienTai = hienTai->next;
        soBuoc++;
    }
    return ketQua;
}

// Xóa theo id
bool xoaTheoID(DanhSach &ds, int id) {
    Node* p = timNodeTheoID(ds, id);
    if (p == nullptr) {
        return false;
    }
    return xoaNode(ds, p);
}

// Đổi chỗ dữ liệu của 2 node
void hoanDoiDuLieu(Node* a, Node* b) {
    if (a == nullptr || b == nullptr) {
        return;
    }
    int idTam = a->id;
    a->id = b->id;
    b->id = idTam;

    std::string tenTam = a->ten;
    a->ten = b->ten;
    b->ten = tenTam;

    double giaTam = a->gia;
    a->gia = b->gia;
    b->gia = giaTam;

    int soLuongTam = a->soLuong;
    a->soLuong = b->soLuong;
    b->soLuong = soLuongTam;
}
