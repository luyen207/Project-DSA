# Module Danh sách liên kết đôi (Node + DanhSach)

Nền tảng dữ liệu cho đồ án **Hệ thống quản lí giỏ hàng online**.
Mỗi mặt hàng trong giỏ là một Node; các Node nối với nhau bằng
danh sách liên kết đôi (có con trỏ `prev` và `next`).

## 1. Cấu trúc file

| File | Nhiệm vụ |
|------|----------|
| `Node.h` | Khai báo `struct Node` và hàm `taoNode` |
| `Node.cpp` | Cài đặt `taoNode` |
| `DoublyLinkedList.h` | Khai báo `struct DanhSach` và các hàm cơ bản |
| `DoublyLinkedList.cpp` | Cài đặt các hàm khởi tạo, kiểm tra, giải phóng |
| `Main.cpp` | Chương trình chạy thử |

## 2. Cấu trúc dữ liệu

### Node

| Thuộc tính | Kiểu | Ý nghĩa |
|-----------|------|---------|
| `id` | `int` | Mã sản phẩm |
| `ten` | `string` | Tên sản phẩm |
| `gia` | `double` | Giá sản phẩm |
| `soLuong` | `int` | Số lượng |
| `prev` | `Node*` | Node đứng trước (node đầu: `nullptr`) |
| `next` | `Node*` | Node đứng sau (node cuối: `nullptr`) |

### DanhSach

| Thuộc tính | Kiểu | Ý nghĩa |
|-----------|------|---------|
| `head` | `Node*` | Node đầu |
| `tail` | `Node*` | Node cuối |
| `soNode` | `int` | Số node hiện có |

## 3. Danh sách hàm

| Hàm | Chức năng | Độ phức tạp |
|-----|-----------|-------------|
| `taoNode` | Tạo node mới; trả về `nullptr` nếu dữ liệu sai | O(1) |
| `khoiTaoDanhSach` | Đưa danh sách về rỗng | O(1) |
| `laRong` | Kiểm tra nhanh `head == nullptr` | O(1) |
| `kiemTraRongHopLe` | Kiểm tra `head`, `tail`, `soNode` có cùng báo rỗng | O(1) |
| `giaiPhongDanhSach` | Xóa mọi node, trả về trạng thái rỗng | O(n) |
| `kiemTraLienKet` | Kiểm tra liên kết đôi đúng quy tắc | O(n) |

### Quy tắc `kiemTraLienKet` kiểm tra

1. `head->prev == nullptr`
2. `tail->next == nullptr`
3. Với mọi node: `node->next->prev == node`
4. Không có vòng lặp (số node duyệt được không vượt `soNode`)
5. Node cuối duyệt được là `tail`, và số node đếm được bằng `soNode`

## 4. Cách build và chạy

Yêu cầu: trình biên dịch `g++` hỗ trợ C++11 trở lên.


g++ Main.cpp Node.cpp DoublyLinkedList.cpp -o test
./test          (Windows PowerShell: .\test)


Kết quả đúng:


Rong: 1
Rong hop le: 1
Lien ket: 1
Node sai la nullptr: 1
Sau khi noi, lien ket: 1
Sau khi pha, lien ket: 0
Sai soNode, lien ket: 0
Sau giai phong, rong: 1


## 5. Quy ước code

- Tên biến, tên hàm: tiếng Việt không dấu, kiểu camelCase.
- Mọi node mới đều có `prev = nullptr`, `next = nullptr`.
- Sau khi dùng xong danh sách phải gọi `giaiPhongDanhSach`.

## 6. Trạng thái

- [x] Khai báo Node, DanhSach
- [x] Tạo node, khởi tạo, kiểm tra rỗng, giải phóng bộ nhớ
- [x] Kiểm tra tính hợp lệ của liên kết
- [ ] Thêm đầu / thêm cuối
- [ ] Xóa node
- [ ] Tìm, sửa node

## 7. Người thực hiện

Họ tên: Nguyễn Văn Luyến   Vai trò: Xây dựng cấu trúc DSLK đôi