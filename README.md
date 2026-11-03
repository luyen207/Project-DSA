# 🛒 HỆ THỐNG GIỎ HÀNG TRỰC TUYẾN

> Đồ án cuối kì môn **Cấu trúc dữ liệu và giải thuật**
> Cấu trúc dữ liệu lõi: **Danh sách liên kết đôi (Doubly Linked List)**

---

## 📑 Mục lục

1. [Thông tin dự án](#1-thông-tin-dự-án)
2. [Giới thiệu sản phẩm](#2-giới-thiệu-sản-phẩm)
3. [Vì sao dùng danh sách liên kết đôi](#3-vì-sao-dùng-danh-sách-liên-kết-đôi)
4. [Chức năng và điểm khác biệt](#4-chức-năng-và-điểm-khác-biệt)
5. [Kiến trúc hệ thống](#5-kiến-trúc-hệ-thống)
6. [Mô hình dữ liệu](#6-mô-hình-dữ-liệu)
7. [Luồng chương trình](#7-luồng-chương-trình)
8. [Cấu trúc thư mục tổng thể](#8-cấu-trúc-thư-mục-tổng-thể)
9. [Bốn phiên bản của dự án](#9-bốn-phiên-bản-của-dự-án)
10. [Quy trình thực hiện](#10-quy-trình-thực-hiện)
11. [Cài đặt, biên dịch và chạy](#11-cài-đặt-biên-dịch-và-chạy)
12. [Định dạng dữ liệu](#12-định-dạng-dữ-liệu)
13. [Kiểm thử](#13-kiểm-thử)
14. [Quản lý rủi ro](#14-quản-lý-rủi-ro)
15. [Phân công thành viên](#15-phân-công-thành-viên)
16. [Chuẩn bị báo cáo và vấn đáp](#16-chuẩn-bị-báo-cáo-và-vấn-đáp)
17. [Hướng phát triển](#17-hướng-phát-triển)
18. [Theo dõi tiến độ](#18-theo-dõi-tiến-độ)

---

## 1. Thông tin dự án

| Mục | Nội dung |
|---|---|
| Tên dự án | Hệ thống giỏ hàng trực tuyến |
| Môn học | Cấu trúc dữ liệu và giải thuật |
| Loại bài | Bài tập lớn cuối kì (dự án nhóm) |
| Yêu cầu bắt buộc | Sử dụng danh sách liên kết đôi (theo yêu cầu giảng viên) |
| Ngôn ngữ | C++ (chuẩn C++11 trở lên) |
| Số thành viên | 5 |
| Nhóm trưởng | _Nguyễn Văn Luyến_ |
| Giảng viên hướng dẫn | _TS.Lê Văn Quốc Anh_ |



### Yêu cầu từ giảng viên

Nhóm phải đáp ứng đủ 4 nội dung khi báo cáo:

1. Giới thiệu và demo sản phẩm.
2. Cho biết chức năng của hệ thống và điểm khác so với các hệ thống giỏ hàng hiện tại.
3. Vấn đáp phần việc của từng thành viên, mỗi người giải thích phần mình làm.
4. Nêu hướng phát triển và nâng cấp sau này.

---

## 2. Giới thiệu sản phẩm

Hệ thống mô phỏng quy trình mua sắm trực tuyến từ lúc **xem sản phẩm**, **thêm vào giỏ**, **chỉnh sửa giỏ** đến **thanh toán** và **xem lịch sử đơn hàng**.

Điểm đặc trưng của dự án là **toàn bộ danh sách trong hệ thống đều được xây dựng trên một lớp danh sách liên kết đôi dùng chung** (`DoublyLinkedList<T>`), thay vì dùng sẵn `vector` hay `list` của thư viện chuẩn. Nhờ đó nhóm tự cài đặt và hiểu rõ từng thao tác con trỏ.

**Mục tiêu sản phẩm**

- Chạy ổn định, không sập khi người dùng nhập sai.
- Dữ liệu không bị mất khi tắt chương trình.
- Thanh toán đúng: không âm kho, không mất đơn, lỗi thì hoàn lại.
- Giao diện dễ dùng, rõ ràng, thao tác ít bước.

**Phạm vi**

- Đây là hệ thống **mô phỏng** phục vụ học tập.
- Thanh toán là **giả lập**, không kết nối ngân hàng hay cổng thanh toán thật.
- Dữ liệu lưu bằng file văn bản (CSV), chưa dùng cơ sở dữ liệu.

---

## 3. Vì sao dùng danh sách liên kết đôi

| Nhu cầu của giỏ hàng | DLL giải quyết thế nào |
|---|---|
| Xóa một món ở giữa giỏ | Có `prev` và `next` nên nối lại hai bên mà không phải dịch cả mảng. Khi đã có con trỏ tới node, thao tác xóa là O(1). |
| Xem giỏ theo hai chiều | Duyệt xuôi bằng `next`, duyệt ngược bằng `prev`. |
| Di chuyển món lên/xuống | Đổi chỗ node với node kế bên bằng cách đổi con trỏ. |
| Undo / Redo | `prev` là lùi một bước, `next` là tiến một bước (kèm con trỏ `current`). |
| Phân trang sản phẩm | Trang trước và trang sau tương ứng `prev` và `next`. |
| Thêm đầu / cuối giỏ | Có cả `head` và `tail` nên cả hai đều O(1). |

**So sánh nhanh (để trả lời vấn đáp)**

| Thao tác | Mảng | Danh sách đơn | Danh sách liên kết đôi |
|---|---|---|---|
| Thêm cuối | O(1) (có thể phải cấp phát lại) | O(1) nếu có `tail` | O(1) |
| Xóa giữa khi đã có con trỏ node | O(n) (dịch phần tử) | O(n) (phải tìm node đứng trước) | **O(1)** |
| Duyệt ngược | Dễ | Không thể | **Dễ** |
| Truy cập theo chỉ số | **O(1)** | O(n) | O(n) |
| Bộ nhớ | Ít nhất | Vừa | Nhiều hơn (thêm `prev`) |

Hạn chế cần nói thật: DLL truy cập theo chỉ số chậm (O(n)) và tốn thêm bộ nhớ cho con trỏ `prev`. Với giỏ hàng, số món thường ít và thao tác chủ yếu là thêm/xóa/duyệt, nên đây là lựa chọn phù hợp.

---

## 4. Chức năng và điểm khác biệt

### 4.1. Danh sách chức năng

| Nhóm | Chức năng | Phiên bản |
|---|---|---|
| Sản phẩm | Xem danh sách sản phẩm | V1 |
| Sản phẩm | Tìm kiếm theo tên | V2 |
| Sản phẩm | Lọc theo danh mục | V2 |
| Sản phẩm | Sắp xếp theo giá (tăng/giảm) | V2 |
| Sản phẩm | Phân trang (trang trước/sau) | V3 |
| Giỏ hàng | Thêm sản phẩm (cộng dồn nếu đã có) | V1 |
| Giỏ hàng | Xóa sản phẩm | V1 |
| Giỏ hàng | Sửa số lượng | V1 |
| Giỏ hàng | Xem giỏ và tính tổng tiền | V1 |
| Giỏ hàng | Di chuyển món lên/xuống | V3 |
| Giỏ hàng | Undo / Redo | V3 |
| Thanh toán | Kiểm tra tồn kho, tạo đơn, in hóa đơn | V2 |
| Thanh toán | Mã giảm giá | V3 |
| Thanh toán | Nhiều phương thức (COD, chuyển khoản, ví giả lập) | V3 |
| Thanh toán | Hoàn tác (rollback) khi lỗi | V3 |
| Đơn hàng | Xem lịch sử đơn hàng | V2 |
| Tài khoản | Đăng ký / đăng nhập, mỗi người một giỏ | V3 |
| Dữ liệu | Lưu và đọc file CSV | V2 |
| Dữ liệu | Sao lưu tự động và khôi phục | V4 |
| Giao diện | Menu console | V1 |
| Giao diện | Giao diện web | V4 |
| Chất lượng | Kiểm tra nhập liệu (Validator) | V2 |
| Chất lượng | Ghi log lỗi | V3 |
| Chất lượng | Bộ test tự động | V1 → V4 |

### 4.2. Điểm khác so với các hệ thống giỏ hàng hiện tại

| Hệ thống thông thường | Hệ thống của nhóm |
|---|---|
| Thường chỉ có thêm/xóa/sửa | Có **Undo/Redo** nhiều bước nhờ DLL |
| Sắp xếp thứ tự món trong giỏ thường cố định | Cho **di chuyển món lên/xuống** tự do |
| Thanh toán lỗi giữa chừng có thể sai kho | Có quy trình **kiểm tra → trừ kho → ghi đơn → rollback nếu lỗi** |
| Cấu trúc bên trong là hộp đen | Mã nguồn lõi **tự cài đặt**, minh bạch, giải thích được từng con trỏ |
| Dữ liệu dễ hỏng khi ghi lỗi | **Sao lưu trước mỗi lần ghi**, khôi phục khi file hỏng (V4) |

> Lưu ý trung thực khi trình bày: các sàn thương mại lớn (Shopee, Lazada...) có quy mô, bảo mật và thanh toán thật mà dự án không có. Điểm khác biệt của nhóm nằm ở **cách tổ chức dữ liệu bằng DLL** và **các tính năng minh họa rõ cấu trúc dữ liệu** (Undo/Redo, di chuyển món, phân trang), không phải ở quy mô.

---

## 5. Kiến trúc hệ thống

Hệ thống chia **4 tầng**. Tầng trên chỉ gọi tầng dưới, không gọi ngược lại.

```
┌─────────────────────────────────────────────┐
│  TẦNG GIAO DIỆN (UI)                        │
│  Menu console (V1-V3)  →  Web UI (V4)       │
└───────────────────┬─────────────────────────┘
                    │ gọi hàm
┌───────────────────▼─────────────────────────┐
│  TẦNG NGHIỆP VỤ (Service)                   │
│  Cart │ Catalog │ OrderService              │
│  PaymentService │ AuthService │ UndoManager │
└───────────────────┬─────────────────────────┘
                    │ thao tác trên
┌───────────────────▼─────────────────────────┐
│  TẦNG CẤU TRÚC DỮ LIỆU (Core) ← TRỌNG TÂM   │
│  Node<T>  +  DoublyLinkedList<T>            │
└───────────────────┬─────────────────────────┘
                    │ đọc/ghi
┌───────────────────▼─────────────────────────┐
│  TẦNG LƯU TRỮ (Storage)                     │
│  File CSV  →  (hướng phát triển: SQL)       │
└─────────────────────────────────────────────┘
```

**Lợi ích của việc phân tầng**

- Nâng từ console (V3) lên web (V4) chỉ đổi tầng UI, không sửa Core.
- Mỗi thành viên làm một tầng/module, ít đụng code của nhau.
- Dễ kiểm thử: test Core độc lập với giao diện.

---

## 6. Mô hình dữ liệu

### 6.1. Node và danh sách liên kết đôi

```
Node<T>:   [prev] ←── [data] ──→ [next]

DoublyLinkedList<T>:
    head   → node đầu
    tail   → node cuối
    size   → số phần tử

Thao tác chính:
    pushFront, pushBack          thêm đầu / cuối
    insertAt                     thêm vào vị trí bất kỳ
    removeNode, removeAt         xóa node
    find                         tìm theo điều kiện
    traverseForward              duyệt xuôi
    traverseBackward             duyệt ngược
    moveUp, moveDown             đổi chỗ với node kế bên
    clear                        xóa toàn bộ, giải phóng bộ nhớ
```

`DoublyLinkedList<T>` viết dưới dạng **template** để dùng chung cho mọi loại dữ liệu, nên chỉ cần một file header.

### 6.2. Các lớp dữ liệu

| Lớp | Thuộc tính chính |
|---|---|
| `Product` | id, ten, gia, tonKho, danhMuc |
| `CartItem` | idSanPham, soLuong, ghiChu |
| `Order` | maDon, tenDangNhap, ngay, DLL<CartItem>, tongTien, trangThai, phuongThuc |
| `User` | tenDangNhap, matKhauBam, hoTen |
| `Voucher` | ma, loai, giaTri, donToiThieu, soLanConLai |
| `CartAction` | loaiThaoTac, idSanPham, soLuongCu, soLuongMoi (dùng cho Undo/Redo) |

### 6.3. Các danh sách trong hệ thống (đều là DLL)

| Danh sách | Kiểu | Dùng để |
|---|---|---|
| Danh mục sản phẩm | `DLL<Product>` | Hiển thị, tìm, lọc, sắp xếp, phân trang |
| Giỏ hàng | `DLL<CartItem>` | Thêm, xóa, sửa, di chuyển món |
| Lịch sử đơn | `DLL<Order>` | Xem đơn cũ mới → cũ hoặc cũ → mới |
| Lịch sử thao tác | `DLL<CartAction>` + con trỏ `current` | Undo / Redo |

---

## 7. Luồng chương trình

### 7.1. Luồng chính

```
[Khởi động]
    │
    ▼
Đọc file dữ liệu → nạp vào DLL ──(lỗi file?)──► báo lỗi + dùng dữ liệu mặc định
    │
    ▼
[Đăng nhập / Khách]  ──────────────────┐
    │                                   │
    ▼                                   │
[MENU CHÍNH] ◄──────────────────────────┘
    ├─ 1. Xem sản phẩm ──► tìm / lọc / sắp xếp / phân trang
    │        └─► chọn món → nhập số lượng → THÊM VÀO GIỎ
    ├─ 2. Xem giỏ hàng ──► sửa SL / xóa / di chuyển / Undo / Redo
    ├─ 3. Thanh toán
    ├─ 4. Lịch sử đơn hàng
    └─ 0. Thoát ──► LƯU file ──► kết thúc
```

### 7.2. Luồng thêm vào giỏ

```
Nhập id + số lượng
   │
   ├─ id không tồn tại?         → báo lỗi, quay lại
   ├─ số lượng ≤ 0 hoặc > tồn?  → báo lỗi, quay lại
   │
   ▼
Duyệt DLL giỏ tìm id đó
   ├─ Đã có   → cộng dồn số lượng (không tạo node mới)
   └─ Chưa có → pushBack node mới
   │
   ▼
Ghi 1 action vào UndoManager  →  hiển thị giỏ mới
```

### 7.3. Luồng thanh toán

```
Giỏ rỗng? ──► chặn, báo "giỏ trống"
   │
   ▼
BƯỚC 1: Kiểm tra lại tồn kho từng món
   │ thiếu hàng → báo món nào thiếu, cho sửa giỏ
   ▼
BƯỚC 2: Áp mã giảm giá (nếu có) → tính tổng tiền
   ▼
BƯỚC 3: Chọn phương thức (COD / chuyển khoản / ví giả lập)
   ▼
BƯỚC 4: Xác nhận cuối (hiện hóa đơn tạm, hỏi Y/N)
   ▼
BƯỚC 5: Trừ tồn kho → tạo Order → ghi file → xóa giỏ
   │ lỗi ở bất kỳ bước ghi nào → ROLLBACK (hoàn tồn kho)
   ▼
In hóa đơn (màn hình + file)
```

**Nguyên tắc cốt lõi:** kiểm tra trước, trừ kho sau, lỗi thì hoàn lại.

### 7.4. Luồng Undo / Redo

```
Các thao tác:   [A] ⇄ [B] ⇄ [C] ⇄ [D]
                              ▲
                           current

Undo  → current lùi về prev  (hoàn tác thao tác C)
Redo  → current tiến lên next (làm lại thao tác C)
Thao tác mới sau khi Undo → xóa các node phía sau current,
                            rồi thêm node mới vào cuối
```

---

## 8. Cấu trúc thư mục tổng thể

Đây là cấu trúc đầy đủ của bản cuối (V4). Phần 9 cho biết mỗi phiên bản thêm những file nào.

```
GioHangOnline/
├── README.md                    # tài liệu này
├── Makefile                     # biên dịch và chạy
│
├── include/                     # file header (.h)
│   ├── core/
│   │   ├── Node.h
│   │   └── DoublyLinkedList.h   # template, dùng cho TẤT CẢ danh sách
│   ├── model/
│   │   ├── Product.h
│   │   ├── CartItem.h
│   │   ├── Order.h
│   │   ├── User.h
│   │   └── Voucher.h
│   ├── service/
│   │   ├── Catalog.h
│   │   ├── Cart.h
│   │   ├── UndoManager.h
│   │   ├── OrderService.h
│   │   ├── PaymentService.h
│   │   └── AuthService.h
│   └── util/
│       ├── FileIO.h
│       ├── Validator.h
│       └── Logger.h
│
├── src/                         # cài đặt (.cpp), song song với include/
│   ├── core/                    # trống: DLL viết trong header (template)
│   ├── model/                   # Product, CartItem, Order, User, Voucher
│   ├── service/                 # Catalog, Cart, UndoManager, OrderService,
│   │                            # PaymentService, AuthService
│   ├── util/                    # FileIO, Validator, Logger
│   ├── ui/
│   │   ├── ConsoleUI.cpp        # V1–V3
│   │   └── WebServer.cpp        # V4
│   └── main.cpp
│
├── web/                         # chỉ dùng ở V4
│   ├── index.html
│   ├── style.css
│   └── app.js
│
├── data/
│   ├── products.csv
│   ├── users.csv
│   ├── vouchers.csv
│   ├── orders.csv
│   ├── carts/                   # mỗi người dùng một file giỏ
│   └── backup/                  # bản sao tự động trước mỗi lần ghi
│
├── tests/
│   ├── test_dll.cpp
│   ├── test_cart.cpp
│   └── test_payment.cpp
│
├── logs/
│   └── app.log
│
└── docs/
    ├── thiet-ke.md              # sơ đồ + giải thích thiết kế
    ├── phan-cong.md             # ai làm file nào (phục vụ vấn đáp)
    └── slide-demo/
```

### Vai trò từng thư mục

| Thư mục | Vai trò |
|---|---|
| `include/` | Khai báo lớp và hàm (file `.h`) |
| `src/` | Cài đặt chi tiết (file `.cpp`) |
| `web/` | Giao diện web (chỉ V4) |
| `data/` | Dữ liệu chạy thật của hệ thống |
| `tests/` | Chương trình kiểm thử |
| `logs/` | Nhật ký lỗi khi chạy |
| `docs/` | Tài liệu thiết kế, phân công, slide |

---

## 9. Bốn phiên bản của dự án

**Nguyên tắc chung:** mỗi phiên bản là một **mốc chạy được độc lập**. Nếu phiên bản sau không kịp thì nhóm vẫn nộp phiên bản trước đó đã hoàn chỉnh.

### Bảng so sánh tổng quát

| | V1 | V2 | V3 | V4 |
|---|---|---|---|---|
| Tên gọi | Nền tảng | Nghiệp vụ | Khác biệt | Hoàn chỉnh |
| DLL + giỏ hàng cơ bản | ✅ | ✅ | ✅ | ✅ |
| Lưu file | ❌ | ✅ | ✅ | ✅ + sao lưu |
| Tìm / lọc / sắp xếp | ❌ | ✅ | ✅ | ✅ |
| Thanh toán | ❌ | Cơ bản | Đầy đủ + rollback | Đầy đủ |
| Undo / Redo | ❌ | ❌ | ✅ | ✅ |
| Tài khoản, voucher | ❌ | ❌ | ✅ | ✅ |
| Giao diện | Console | Console | Console | **Web** (kèm console dự phòng) |
| Kiểm thử | Test DLL | + Cart | + Payment | Đủ bộ |

---

### 🔹 V1: Nền tảng

**Mục tiêu:** DLL chạy đúng và giỏ hàng cơ bản dùng được.

**Chức năng**

- Cài đặt `DoublyLinkedList<T>`: `pushBack`, `removeNode`, `find`, duyệt hai chiều.
- Giỏ hàng: thêm, xóa, sửa số lượng, xem, tính tổng tiền.
- Danh mục sản phẩm khởi tạo cứng trong code (khoảng 10–15 sản phẩm).
- Menu console đơn giản. **Chưa lưu file.**

**File của V1**

```
GioHangOnline/
├── README.md
├── Makefile
├── include/
│   ├── core/
│   │   ├── Node.h
│   │   └── DoublyLinkedList.h
│   ├── model/
│   │   ├── Product.h
│   │   └── CartItem.h
│   └── service/
│       └── Cart.h
├── src/
│   ├── model/                (Product.cpp, CartItem.cpp)
│   ├── service/              (Cart.cpp)
│   ├── ui/ConsoleUI.cpp
│   └── main.cpp
└── tests/
    └── test_dll.cpp
```

**Quá trình thực hiện**

| Bước | Công việc | Người phụ trách |
|---|---|---|
| 1 | Viết `Node.h` và `DoublyLinkedList.h` (thêm/xóa/duyệt) | Nhóm trưởng |
| 2 | Viết `test_dll.cpp`: danh sách rỗng, 1 node, thêm/xóa đầu, giữa, cuối | Nhóm trưởng |
| 3 | Viết `Product`, `CartItem` | TV 3, TV 2 |
| 4 | Viết `Cart` dùng DLL | TV 2 |
| 5 | Viết menu console và `main.cpp` | TV 5 |
| 6 | Tích hợp, chạy thử, sửa lỗi | Cả nhóm |

**Tiêu chí hoàn thành V1**

- [ ] Test DLL qua hết các trường hợp biên.
- [ ] Demo thêm/xóa/sửa giỏ liên tục 5 phút không sập.
- [ ] Mỗi thành viên giải thích được phần của mình.

---

### 🔹 V2: Hoàn thiện nghiệp vụ

**Mục tiêu:** hệ thống dùng được thật, dữ liệu không mất, nhập sai không sập.

**Chức năng thêm so với V1**

- Lưu và đọc dữ liệu bằng file CSV.
- Tìm kiếm theo tên, lọc theo danh mục.
- Sắp xếp theo giá (merge sort trên DLL).
- Thanh toán cơ bản: kiểm tra tồn kho, tạo đơn, in hóa đơn.
- Lịch sử đơn hàng.
- Validator: chặn nhập chữ thay số, số âm, id không tồn tại.

**File thêm ở V2**

```
include/
├── model/Order.h
├── service/
│   ├── Catalog.h
│   └── OrderService.h
└── util/
    ├── FileIO.h
    └── Validator.h
src/
├── model/Order.cpp
├── service/(Catalog.cpp, OrderService.cpp)
└── util/(FileIO.cpp, Validator.cpp)
data/
├── products.csv
└── orders.csv
tests/
└── test_cart.cpp
```

**Quá trình thực hiện**

| Bước | Công việc | Người phụ trách |
|---|---|---|
| 1 | Viết `FileIO` đọc/ghi CSV, nạp vào DLL | TV 5 |
| 2 | Chuyển danh mục từ code cứng sang `products.csv` | TV 3 |
| 3 | Viết tìm / lọc / sắp xếp | TV 3 |
| 4 | Viết `Validator` và gắn vào mọi chỗ nhập | TV 5 |
| 5 | Viết `Order`, `OrderService` (kiểm kho, tạo đơn, hóa đơn) | TV 4 |
| 6 | Viết lịch sử đơn hàng | TV 4 |
| 7 | Viết `test_cart.cpp` | TV 2 |
| 8 | Tích hợp, kiểm thử nhập bậy | Cả nhóm |

**Tiêu chí hoàn thành V2**

- [ ] Tắt và mở lại chương trình vẫn còn dữ liệu.
- [ ] Nhập bậy (chữ, số âm, id lạ) không làm chương trình sập.
- [ ] Thanh toán không bao giờ làm tồn kho âm.

---

### 🔹 V3: Khác biệt

**Mục tiêu:** có những tính năng làm nên điểm khác biệt, chuẩn bị cho phần giảng viên hỏi "khác gì hệ thống hiện tại".

**Chức năng thêm so với V2**

- **Undo / Redo** giỏ hàng (DLL + con trỏ `current`).
- **Di chuyển món** lên/xuống trong giỏ.
- **Phân trang** sản phẩm (trang trước/sau).
- **Tài khoản:** đăng ký, đăng nhập, mỗi người một giỏ.
- **Mã giảm giá** và nhiều phương thức thanh toán (giả lập).
- **Rollback** khi thanh toán lỗi, ghi log lỗi ra file.

**File thêm ở V3**

```
include/
├── model/
│   ├── User.h
│   └── Voucher.h
├── service/
│   ├── UndoManager.h
│   ├── PaymentService.h
│   └── AuthService.h
└── util/Logger.h
src/
├── model/(User.cpp, Voucher.cpp)
├── service/(UndoManager.cpp, PaymentService.cpp, AuthService.cpp)
└── util/Logger.cpp
data/
├── users.csv
├── vouchers.csv
└── carts/                  # mỗi người dùng một file giỏ
logs/
└── app.log
tests/
└── test_payment.cpp
```

**Quá trình thực hiện**

| Bước | Công việc | Người phụ trách |
|---|---|---|
| 1 | Viết `UndoManager` (DLL `CartAction` + `current`) | TV 2 |
| 2 | Thêm `moveUp`/`moveDown` vào DLL và giỏ hàng | Nhóm trưởng, TV 2 |
| 3 | Viết phân trang cho danh mục | TV 3 |
| 4 | Viết `User`, `AuthService`, lưu giỏ theo từng người | TV 5 |
| 5 | Viết `Voucher`, `PaymentService` (nhiều phương thức) | TV 4 |
| 6 | Thêm rollback vào thanh toán | TV 4 |
| 7 | Viết `Logger` và gắn vào các điểm dễ lỗi | TV 5 |
| 8 | Viết `test_payment.cpp` | TV 4 |
| 9 | Tích hợp, kiểm thử, sửa lỗi | Cả nhóm |

**Tiêu chí hoàn thành V3**

- [ ] Undo/Redo đúng qua nhiều bước, kể cả khi thao tác mới sau Undo.
- [ ] Giả lập lỗi thanh toán vẫn hoàn tồn kho đúng.
- [ ] Hai tài khoản khác nhau có giỏ riêng, không lẫn.

---

### 🔹 V4: Bản hoàn chỉnh (final)

**Mục tiêu:** chất lượng cao, giao diện hiện đại, sẵn sàng báo cáo.

**Chức năng thêm so với V3**

- **Giao diện web:** C++ chạy server nhỏ (thư viện `cpp-httplib`, một file header), giao diện HTML/CSS/JS gọi API.
- Giỏ hàng cập nhật ngay trên trang, hiển thị rõ ràng, tương thích màn hình điện thoại.
- **Sao lưu tự động** trước mỗi lần ghi, khôi phục khi file hỏng.
- Bộ test đầy đủ cho DLL, giỏ hàng, thanh toán.
- Tài liệu, slide và kịch bản demo hoàn chỉnh.
- Giữ **chế độ console dự phòng** nếu web gặp sự cố ngày báo cáo.

**File thêm ở V4**

```
src/ui/WebServer.cpp
web/
├── index.html
├── style.css
└── app.js
data/backup/                # bản sao tự động
docs/
├── thiet-ke.md
├── phan-cong.md
└── slide-demo/
```

**Quá trình thực hiện**

| Bước | Công việc | Người phụ trách |
|---|---|---|
| 1 | Thiết kế các API (xem sản phẩm, giỏ, thanh toán...) | Nhóm trưởng |
| 2 | Viết `WebServer.cpp` nối API với Service | TV 5, Nhóm trưởng |
| 3 | Viết giao diện web (`index.html`, `style.css`, `app.js`) | TV 5, TV 3 |
| 4 | Thêm sao lưu và khôi phục vào `FileIO` | TV 5 |
| 5 | Bổ sung test, chạy toàn bộ bộ test | Cả nhóm |
| 6 | Viết `thiet-ke.md`, `phan-cong.md`, làm slide | Cả nhóm |
| 7 | Tập demo, quay video demo dự phòng | Cả nhóm |

**Tiêu chí hoàn thành V4**

- [ ] Demo trên trình duyệt chạy trơn tru, không lỗi.
- [ ] Chế độ console dự phòng vẫn chạy được.
- [ ] Toàn bộ test qua.
- [ ] Có video demo dự phòng.
- [ ] Mỗi thành viên trả lời được câu hỏi về phần của mình.

> **Cảnh báo rủi ro V4:** phần web là phần rủi ro nhất. Vì vậy nó nằm ở V4 và **không làm thay đổi Core**. Nếu không kịp, nhóm nộp V3 vẫn đầy đủ điểm khác biệt.

---

## 10. Quy trình thực hiện

### 10.1. Quy trình tổng thể

```
Họp nhóm chốt ngôn ngữ + phân công
        │
        ▼
Viết Core (DLL) + test  ──►  V1  ──►  Tích hợp, chạy thử
        │
        ▼
Thêm nghiệp vụ, lưu file  ──►  V2  ──►  Tích hợp, kiểm thử nhập bậy
        │
        ▼
Thêm tính năng khác biệt  ──►  V3  ──►  Tích hợp, kiểm thử
        │
        ▼
Thêm web, sao lưu, tài liệu  ──►  V4  ──►  Demo thử, hoàn thiện
        │
        ▼
Báo cáo và vấn đáp
```

### 10.2. Quy tắc làm việc nhóm

1. **Mỗi người sở hữu theo module**, tự viết và tự giải thích phần của mình.
2. **Core làm trước và test kỹ**, vì cả nhóm phụ thuộc vào đó.
3. **Tích hợp cuối mỗi phiên bản**, không dồn lỗi tích hợp về cuối kỳ.
4. **Đọc chéo code** (code review) khoảng 15 phút cho mỗi module, để ít nhất hai người hiểu mỗi phần.
5. **Không nộp code không hiểu.** Vấn đáp sẽ lộ ngay phần nào chép mà không hiểu.

### 10.3. Quy trình Git (khuyến nghị)

```
main            ← bản ổn định, chỉ ghép khi đã test
 ├─ dev         ← nhánh tích hợp chung
 │   ├─ tv1-core
 │   ├─ tv2-cart
 │   ├─ tv3-catalog
 │   ├─ tv4-order
 │   └─ tv5-storage-ui
```

- Mỗi người làm trên nhánh của mình.
- Ghép vào `dev` khi xong một phần và đã chạy test.
- Ghép `dev` vào `main` khi hoàn thành mỗi phiên bản, rồi gắn tag `v1`, `v2`, `v3`, `v4`.

### 10.4. Quy ước đặt tên

- Tên lớp: `PascalCase` (ví dụ `CartItem`).
- Tên hàm và biến: `camelCase` (ví dụ `themVaoGio`, `tongTien`).
- Một lớp một cặp file `.h` và `.cpp` trùng tên.
- Có comment ngắn giải thích ở các đoạn xử lý con trỏ.

---

## 11. Cài đặt, biên dịch và chạy

### Yêu cầu

- Trình biên dịch hỗ trợ C++11 trở lên (g++, clang++ hoặc MSVC).
- `make` (hoặc dùng Code::Blocks / Visual Studio / CMake nếu nhóm quen hơn).

### Các lệnh (khi đã viết `Makefile`)

```bash
# Biên dịch
make

# Chạy chương trình (console)
make run

# Chạy bộ test
make test

# Dọn file biên dịch
make clean
```

> Nội dung `Makefile` sẽ được viết khi làm V1. Các lệnh trên là mục tiêu cần hỗ trợ.

### Chạy bản web (V4)

```bash
make web
# Mở trình duyệt tại địa chỉ hiển thị trên màn hình (ví dụ http://localhost:8080)
```

### Lưu ý khi nộp / demo

- Chép cả thư mục `data/` kèm chương trình để có dữ liệu mẫu.
- Kiểm tra chương trình chạy được trên **máy dùng để demo**, không chỉ trên máy cá nhân.
- Chuẩn bị sẵn video demo phòng khi máy hoặc mạng có sự cố.

---

## 12. Định dạng dữ liệu

Dữ liệu lưu dạng CSV, mỗi dòng một bản ghi, ngăn cách bằng dấu phẩy. Dòng đầu là tiêu đề.

**`data/products.csv`**
```
id,ten,gia,tonKho,danhMuc
```

**`data/users.csv`**
```
tenDangNhap,matKhauBam,hoTen
```
> Không lưu mật khẩu dạng chữ thường. Lưu **giá trị đã băm**.

**`data/vouchers.csv`**
```
maVoucher,loai,giaTri,donToiThieu,soLanConLai
```

**`data/orders.csv`**
```
maDon,tenDangNhap,ngay,tongTien,trangThai,phuongThuc,danhSachMon
```
`danhSachMon` có dạng `id:soLuong;id:soLuong`.

**`data/carts/<tenDangNhap>.csv`**
```
idSanPham,soLuong,ghiChu
```

**Quy tắc ghi file an toàn**

1. Sao chép file cũ vào `data/backup/` trước khi ghi.
2. Ghi ra file tạm, ghi xong mới thay file thật.
3. Khi đọc, nếu file hỏng thì thông báo rõ và khôi phục từ bản sao lưu gần nhất.

---

## 13. Kiểm thử

### 13.1. Các lớp kiểm thử

| File test | Kiểm tra | Từ phiên bản |
|---|---|---|
| `test_dll.cpp` | DLL: rỗng, 1 node, thêm/xóa đầu-giữa-cuối, duyệt xuôi/ngược, `moveUp`/`moveDown`, `clear` | V1 |
| `test_cart.cpp` | Thêm cộng dồn, xóa, sửa số lượng, tổng tiền, Undo/Redo | V2 (mở rộng V3) |
| `test_payment.cpp` | Thiếu kho, giỏ rỗng, voucher, rollback khi lỗi | V3 |

### 13.2. Các trường hợp biên của DLL cần test

- [ ] Danh sách rỗng: duyệt, xóa, tìm.
- [ ] Danh sách có đúng 1 phần tử rồi xóa nó (`head` và `tail` phải về rỗng).
- [ ] Xóa node đầu, node cuối, node giữa.
- [ ] Thêm vào vị trí đầu, cuối, giữa, vị trí ngoài phạm vi.
- [ ] `moveUp` ở node đầu, `moveDown` ở node cuối (không được hỏng danh sách).
- [ ] Duyệt xuôi và duyệt ngược cho cùng kết quả (đảo thứ tự).
- [ ] `clear` rồi dùng lại danh sách.

### 13.3. Kiểm thử nhập liệu

- [ ] Nhập chữ ở chỗ cần số.
- [ ] Nhập số âm, số 0, số rất lớn.
- [ ] Nhập id không tồn tại.
- [ ] Nhập chuỗi rỗng, chuỗi rất dài.
- [ ] Nhấn Enter liên tục.

### 13.4. Kiểm thử bộ nhớ

- Kiểm tra rò rỉ bộ nhớ (ví dụ dùng `valgrind` trên Linux hoặc công cụ tương đương), nhất là sau khi xóa node và `clear`.

---

## 14. Quản lý rủi ro

### 14.1. Các phương án triển khai đã cân nhắc

| Phương án | Mô tả | Khả thi (ước lượng) | Rủi ro chính |
|---|---|---|---|
| **A: Đi từng bậc V1 → V4 (chọn)** | Mỗi V là một mốc nộp được | ~85–90% | V4 web có thể không kịp, khi đó nộp V3 |
| B: Chỉ console, dồn sức cho V3 | Bỏ web | ~90–95% | Giao diện ít ấn tượng |
| C: Làm thẳng bản web | Bỏ qua V1–V2 | ~50–60% | Kẹt tích hợp, DLL chưa vững |
| D: Chia 5 mảng riêng, ghép cuối | Ghép ở tuần cuối | ~55–65% | Lỗi tích hợp dồn vào phút chót |

> Tỉ lệ là **ước lượng** dựa trên độ phức tạp và trình độ nhóm, không phải số đo chính xác.

**Lý do chọn phương án A (đạt ngưỡng ≥ 80%)**

1. Mỗi mốc đều nộp được, luôn có phương án lùi.
2. Core làm trước và test kỹ, các tầng trên có nền chắc.
3. Tích hợp liên tục, không dồn về cuối.

### 14.2. Bảng rủi ro và cách xử lý

| Rủi ro | Mức độ | Cách xử lý |
|---|---|---|
| Lỗi con trỏ / rò rỉ bộ nhớ trong DLL | Cao | Viết `test_dll.cpp` trước, test mọi trường hợp biên |
| Thành viên không kịp hoặc không hiểu bài | Cao | Chia module nhỏ, đọc chéo code, hỏi nhau trong nhóm |
| Nhập sai làm sập chương trình | Cao | Validator cho **mọi** chỗ nhập |
| Lỗi tích hợp dồn cuối kỳ | Trung bình | Tích hợp cuối mỗi phiên bản |
| Mất hoặc hỏng dữ liệu | Trung bình | Sao lưu trước khi ghi, ghi qua file tạm |
| Web hỏng ngày demo | Trung bình | Giữ console dự phòng, quay video demo |
| Xung đột code khi làm chung | Trung bình | Dùng Git, mỗi người một nhánh |
| Thanh toán sai kho | Cao | Kiểm tra → trừ → ghi → rollback |
| Không đủ thời gian | Trung bình | Cắt tính năng theo thứ tự: web (V4) trước, rồi voucher, rồi phân trang |

---

## 15. Phân công thành viên

| TV | Sở hữu (file) | Câu hỏi vấn đáp có thể gặp |
|---|---|---|
| **1 (Nhóm trưởng)** | `Node.h`, `DoublyLinkedList.h`, `test_dll.cpp`, `main.cpp`, tích hợp | Vì sao dùng DLL? Độ phức tạp từng thao tác? Xử lý con trỏ khi xóa node ra sao? |
| **2** | `Cart`, `CartItem`, `UndoManager`, `test_cart.cpp` | Thêm món đã có thì xử lý thế nào? Undo/Redo hoạt động ra sao? |
| **3** | `Product`, `Catalog`, tìm/lọc/sắp xếp, phân trang | Sắp xếp trên DLL làm thế nào? Phân trang dùng `prev`/`next` ra sao? |
| **4** | `Order`, `OrderService`, `PaymentService`, `Voucher`, `test_payment.cpp` | Vì sao kiểm tra kho trước khi trừ? Rollback là gì, làm thế nào? |
| **5** | `FileIO`, `Validator`, `Logger`, `User`, `AuthService`, giao diện | Dữ liệu lưu thế nào? Nhập sai xử lý ra sao? Mật khẩu lưu thế nào? |

Chi tiết phân công thực tế ghi tại `docs/phan-cong.md` (mỗi người ghi rõ file mình viết và hàm quan trọng nhất).

**Mẫu ghi trong `docs/phan-cong.md`**

```
Thành viên: ...
File phụ trách: ...
Hàm quan trọng nhất: ...
Giải thích ngắn (3–5 câu) hàm đó hoạt động thế nào: ...
Khó khăn gặp phải và cách giải quyết: ...
```

---

## 16. Chuẩn bị báo cáo và vấn đáp

### 16.1. Bố cục buổi báo cáo (gợi ý)

| Phần | Nội dung | Gợi ý thời lượng |
|---|---|---|
| 1 | Giới thiệu sản phẩm và mục tiêu | 1–2 phút |
| 2 | Demo luồng chính: xem sản phẩm → thêm giỏ → sửa → thanh toán | 3–5 phút |
| 3 | Demo điểm khác biệt: Undo/Redo, di chuyển món, rollback | 2–3 phút |
| 4 | Giải thích cấu trúc DLL và vì sao chọn | 2 phút |
| 5 | Mỗi thành viên nói phần mình | 1–2 phút/người |
| 6 | Hướng phát triển | 1 phút |

### 16.2. Checklist trước ngày báo cáo

- [ ] Chương trình chạy được trên máy demo.
- [ ] Dữ liệu mẫu trong `data/` đã đầy đủ.
- [ ] Có kịch bản demo từng bước, đã tập ít nhất 2 lần.
- [ ] Có video demo dự phòng.
- [ ] Chế độ console dự phòng chạy được.
- [ ] Mỗi người đã đọc lại code phần mình và phần của một người khác.
- [ ] Slide đã xong, thống nhất người trình bày từng phần.

### 16.3. Câu hỏi thường gặp và hướng trả lời

| Câu hỏi | Ý chính cần nói |
|---|---|
| Vì sao dùng DLL mà không dùng mảng? | Xóa giữa O(1) khi có con trỏ, duyệt hai chiều, Undo/Redo tự nhiên. Nói thêm hạn chế: truy cập theo chỉ số O(n), tốn bộ nhớ thêm. |
| Độ phức tạp thêm vào cuối? | O(1) vì có `tail`. |
| Tìm một sản phẩm trong giỏ? | O(n), phải duyệt từ `head`. |
| Khi xóa node cuối, cần cập nhật gì? | `tail` trỏ về node trước, `next` của node mới cuối đặt về `nullptr`, giảm `size`. |
| Xóa phần tử duy nhất thì sao? | `head` và `tail` đều về `nullptr`, `size` về 0. |
| Undo/Redo làm thế nào? | DLL các thao tác + con trỏ `current`; thao tác mới sau Undo sẽ cắt phần phía sau. |
| Thanh toán lỗi giữa chừng thì sao? | Kiểm tra trước, trừ kho sau, lỗi thì rollback hoàn kho. |
| Dữ liệu lưu thế nào, mất thì sao? | CSV, sao lưu trước khi ghi, khôi phục từ bản sao. |
| Khác hệ thống hiện tại ở đâu? | Undo/Redo, di chuyển món, rollback minh bạch, lõi tự cài đặt. Nói rõ quy mô không so được với sàn lớn. |
| Hướng phát triển? | Xem phần 17. |

---

## 17. Hướng phát triển

| Hướng | Nội dung |
|---|---|
| Cơ sở dữ liệu | Thay CSV bằng SQL (ví dụ MySQL), giữ DLL làm bộ nhớ đệm |
| Thanh toán thật | Kết nối cổng thanh toán (VNPay, MoMo...) thay cho giả lập |
| Gợi ý sản phẩm | Dựa trên lịch sử mua hàng (hướng dữ liệu / AI) |
| Đồng bộ | Giỏ hàng đồng bộ nhiều thiết bị theo tài khoản |
| Quản trị | Thêm/sửa/xóa sản phẩm, thống kê doanh thu |
| Bảo mật | Băm mật khẩu mạnh hơn, phân quyền người dùng |
| Ứng dụng di động | Tách lõi thành API để làm app mobile |
| Hiệu năng | Thêm bảng băm để tìm sản phẩm nhanh hơn khi dữ liệu lớn |

---

## 18. Theo dõi tiến độ

| Phiên bản | Trạng thái | Ngày bắt đầu | Ngày xong | Ghi chú |
|---|---|---|---|---|
| V1: Nền tảng | ⬜ Chưa bắt đầu | | | |
| V2: Nghiệp vụ | ⬜ Chưa bắt đầu | | | |
| V3: Khác biệt | ⬜ Chưa bắt đầu | | | |
| V4: Hoàn chỉnh | ⬜ Chưa bắt đầu | | | |

Ký hiệu: ⬜ chưa bắt đầu · 🟨 đang làm · ✅ hoàn thành · ❌ bỏ (ghi lý do)

---

## 📌 Ghi chú cuối

- Cập nhật README này khi hoàn thành mỗi phiên bản (tích các ô tiêu chí, cập nhật bảng tiến độ).
- Bất cứ thay đổi nào về cấu trúc file cần cập nhật ở **phần 8** và **phần 9**.
- Các mục ghi "_(điền)_" cần nhóm bổ sung thông tin thật trước khi nộp.
