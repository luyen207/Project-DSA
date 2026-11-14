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
| `../tests/test_doubly_linked_list.cpp` | Chương trình kiểm thử (53 test) |

## 2. Cấu trúc dữ liệu

### Node

| Thuộc tính | Kiểu | Ý nghĩa |
|-----------|------|---------|
| `id` | `int` | Mã sản phẩm (> 0) |
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
| `taoNode` | Tạo node mới; trả về `nullptr` nếu dữ liệu sai (id > 0, tên không rỗng, 0 ≤ giá ≤ `GIA_TOI_DA`, 1 ≤ số lượng ≤ `SO_LUONG_TOI_DA`) | O(1) |
| `khoiTaoDanhSach` | Đưa danh sách về rỗng | O(1) |
| `laRong` | Kiểm tra nhanh `head == nullptr` | O(1) |
| `kiemTraRongHopLe` | Kiểm tra `head`, `tail`, `soNode` có cùng báo rỗng | O(1) |
| `giaiPhongDanhSach` | Xóa mọi node, trả về trạng thái rỗng | O(n) |
| `thuocDanhSach` | Kiểm tra node `p` có nằm trong danh sách không | O(n) |
| `kiemTraLienKet` | Kiểm tra liên kết đôi đúng quy tắc | O(n) |
| `themDau` | Thêm node vào đầu danh sách | O(1) |
| `themCuoi` | Thêm node vào cuối danh sách | O(1) |
| `chenSau` | Chèn node mới ngay sau node `p` (từ chối nếu `p` không thuộc danh sách) | O(n) |
| `xoaNode` | Xóa node `p` (đủ 4 trường hợp, từ chối nếu `p` không thuộc danh sách) | O(n) |
| `timNodeTheoID` | Tìm node theo id, không thấy trả về `nullptr` | O(n) |
| `xoaTheoID` | Tìm rồi xóa node theo id | O(n) |
| `hoanDoiDuLieu` | Đổi dữ liệu 2 node, không đổi con trỏ (dùng cho sắp xếp) | O(1) |

### Ví dụ sử dụng (cho TV2, TV3, TV4)

```cpp
#include "DoublyLinkedList.h"

DanhSach ds;
khoiTaoDanhSach(ds);

themCuoi(ds, taoNode(1, "Ao", 100000, 2));   // taoNode sai du lieu -> nullptr -> themCuoi tra ve false
Node* p = timNodeTheoID(ds, 1);              // nullptr neu khong co
xoaTheoID(ds, 1);                            // false neu khong co id do

giaiPhongDanhSach(ds);                       // luon goi khi dung xong
```

### Lưu ý khi dùng
- `DanhSach` **tự rỗng khi tạo** và **tự giải phóng khi ra khỏi phạm vi** (destructor). Vẫn có thể gọi `giaiPhongDanhSach` sớm.
- `DanhSach` **không được sao chép** (`DanhSach b = a;` báo lỗi biên dịch) → truyền bằng tham chiếu `DanhSach&`.
- `xoaNode` và `chenSau`: nếu `p` không thuộc danh sách thì trả về `false`, danh sách không đổi.
- Giới hạn dữ liệu: `GIA_TOI_DA` = 1.000 tỷ, `SO_LUONG_TOI_DA` = 1.000.000 (khai báo trong `Node.h`).
- `xoaNode` đã `delete` node, **không dùng lại `p`** sau khi xóa.
- Muốn sắp xếp: dùng `hoanDoiDuLieu(a, b)` thay vì tự đổi `prev/next`.
- Nên gọi `kiemTraLienKet(ds)` sau các thao tác lớn để phát hiện lỗi sớm.

### Quy tắc `kiemTraLienKet` kiểm tra

1. `head->prev == nullptr`
2. `tail->next == nullptr`
3. Với mọi node: `node->next->prev == node`
4. Không có vòng lặp (số node duyệt được không vượt `soNode`)
5. Node cuối duyệt được là `tail`, và số node đếm được bằng `soNode`

## 4. Cách build và chạy

Yêu cầu: `g++` hỗ trợ C++17 (hoặc dùng CMake như dưới).

**Build riêng module này** (từ thư mục `Backend/`):

```
g++ -std=c++17 -ICommon tests/test_doubly_linked_list.cpp Common/Node.cpp Common/DoublyLinkedList.cpp -o test
./test            (Windows: test.exe)
```

**Build cả dự án:**

```
# Tu thu muc Backend/ (can cmake + g++/clang)
cmake -S Integration -B build -DSHOPPRO_SANITIZE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
(Windows/MinGW: bo `-DSHOPPRO_SANITIZE=ON` vi MinGW thuong khong co AddressSanitizer.)

**Kết quả đúng:** tất cả dòng đều `[PASS]`, dòng cuối là:

```
KET QUA: 53/53 test dat
```

## 5. Quy ước code

- Tên biến, tên hàm: tiếng Việt không dấu, kiểu camelCase.
- Mọi node mới đều có `prev = nullptr`, `next = nullptr`.
- Sau khi dùng xong danh sách phải gọi `giaiPhongDanhSach`.

## 6. Trạng thái

- [x] Khai báo Node, DanhSach
- [x] Tạo node, khởi tạo, kiểm tra rỗng, giải phóng bộ nhớ
- [x] Kiểm tra tính hợp lệ của liên kết
- [x] Thêm đầu / thêm cuối / chèn sau
- [x] Xóa node (đầu, cuối, giữa, node duy nhất), xóa theo id
- [x] Tìm node theo id
- [x] Hoán đổi dữ liệu 2 node (phục vụ sắp xếp)
- [x] Kiểm thử 53 trường hợp
- [x] Sau review: từ chối node lạ, kiểm tra chặt dữ liệu, tự giải phóng (RAII), cấm sao chép

## 7. Người thực hiện

Họ tên: Nguyễn Văn Luyến   Vai trò: Xây dựng cấu trúc DSLK đôi
