#include "DoublyLinkedList.h"

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