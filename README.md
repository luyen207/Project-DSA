
# HỆ THỐNG QUẢN LÝ GIỎ HÀNG ONLINE
### Online Shopping Cart Management System

> Dự án học tập về cấu trúc dữ liệu và giải thuật, ứng dụng danh sách liên kết đôi (Doubly Linked List) trong quản lý giỏ hàng.

---

## 1. Giới thiệu dự án

Hệ thống quản lý giỏ hàng online là dự án xây dựng chương trình quản lý sản phẩm trong giỏ hàng. Backend sử dụng C++ và cấu trúc dữ liệu Danh sách liên kết đôi (Doubly Linked List). Frontend sử dụng HTML, CSS và JavaScript.

Hệ thống hướng đến các chức năng quản lý sản phẩm, tìm kiếm, hiển thị, tính toán tổng giá trị giỏ hàng và sắp xếp sản phẩm.

## 2. Mục tiêu dự án

- Vận dụng cấu trúc dữ liệu danh sách liên kết đôi vào bài toán thực tế.
- Xây dựng chức năng thêm, xóa và cập nhật sản phẩm.
- Tìm kiếm và hiển thị danh sách sản phẩm.
- Tính tổng số lượng và tổng giá trị giỏ hàng.
- Sắp xếp sản phẩm theo giá.
- Xây dựng giao diện quản lý giỏ hàng.
- Thực hành làm việc nhóm và quản lý mã nguồn bằng Git/GitHub.

## 3. Công nghệ sử dụng

| Thành phần | Công nghệ |
|---|---|
| Backend | C++ |
| Frontend | HTML5, CSS3, JavaScript |
| Cấu trúc dữ liệu | Doubly Linked List |
| Quản lý mã nguồn | Git, GitHub |
| Trình biên dịch | G++ / C++ Compiler |
| Build system | CMake (nếu được cấu hình) |

## 4. Đặc tả chức năng

### 4.1. Quản lý sản phẩm

- Thêm sản phẩm vào giỏ hàng.
- Xóa sản phẩm khỏi giỏ hàng.
- Cập nhật giá sản phẩm.
- Cập nhật số lượng sản phẩm.
- Kiểm tra dữ liệu đầu vào.
- Kiểm tra mã sản phẩm trùng lặp.

### 4.2. Tìm kiếm và hiển thị

- Tìm kiếm sản phẩm theo ID.
- Hiển thị danh sách sản phẩm.
- Duyệt danh sách theo chiều thuận.
- Duyệt danh sách theo chiều ngược.

### 4.3. Xử lý giỏ hàng

- Tính tổng số lượng sản phẩm.
- Tính tổng giá trị giỏ hàng.
- Sắp xếp sản phẩm theo giá.
- Kiểm tra kết quả sau khi sắp xếp.

### 4.4. Giao diện người dùng

- Trang tổng quan (Dashboard).
- Trang danh sách sản phẩm.
- Biểu mẫu thêm và cập nhật sản phẩm.
- Chức năng tìm kiếm.
- Khu vực hiển thị tổng tiền và tổng số lượng.

## 5. Cấu trúc dữ liệu

Dự án sử dụng **Danh sách liên kết đôi (Doubly Linked List)**.

Mỗi node lưu thông tin một sản phẩm và hai con trỏ liên kết với node trước, node sau.

| Thuộc tính | Kiểu dữ liệu | Ý nghĩa |
|---|---|---|
| id | int | Mã sản phẩm |
| ten | string | Tên sản phẩm |
| gia | double | Giá sản phẩm |
| soLuong | int | Số lượng |
| prev | Node* | Node đứng trước |
| next | Node* | Node đứng sau |

### Mô hình danh sách liên kết đôi

text
HEAD
  |
  v
NULL <- [Node 1] <-> [Node 2] <-> [Node 3] -> NULL
                                      ^
                                      |
                                     TAIL


### Nguyên tắc quản lý

- `head` trỏ đến node đầu tiên.
- `tail` trỏ đến node cuối cùng.
- `prev` liên kết đến node đứng trước.
- `next` liên kết đến node đứng sau.
- Danh sách rỗng: `head == nullptr` và `tail == nullptr`.
- Node đầu tiên có `prev == nullptr`.
- Node cuối cùng có `next == nullptr`.
- Phải giải phóng bộ nhớ khi xóa node hoặc hủy danh sách.

## 6. Kiến trúc tổng thể hệ thống

mermaid
flowchart TB
    U["Người sử dụng"]
    FE["Frontend<br/>HTML - CSS - JavaScript"]
    API["Lớp giao tiếp/API<br/>HTTP và JSON"]
    BE["Backend C++"]
    DLL["Doubly Linked List<br/>Node - head - tail"]

    PM["Quản lý sản phẩm<br/>Thêm - Xóa - Cập nhật"]
    PS["Tìm kiếm và hiển thị<br/>Tìm kiếm - Duyệt xuôi/ngược"]
    CP["Xử lý giỏ hàng<br/>Tính tổng - Sắp xếp"]

    U --> FE
    FE <--> API
    API <--> BE
    BE <--> DLL
    DLL --> PM
    DLL --> PS
    DLL --> CP


**Lưu ý:** Frontend JavaScript không gọi trực tiếp các hàm C++ thông thường. Nếu triển khai kết nối thực tế, hệ thống cần lớp trung gian/API hoặc một phương án giao tiếp tương đương. Sơ đồ trên thể hiện kiến trúc mục tiêu, không khẳng định API đã được triển khai.
##7. Cấu trúc thư mục dự án
```text
OnlineShoppingCart/
│
├── README.md
│
├── Backend/
│   │
│   ├── Common/
│   │   ├── Node.h
│   │   ├── Node.cpp
│   │   ├── DoublyLinkedList.h
│   │   ├── DoublyLinkedList.cpp
│   │   └── README.md
│   │
│   ├── Member_03_ProductManager/
│   │   ├── ProductManager.h
│   │   ├── ProductManager.cpp
│   │   ├── ProductValidation.h
│   │   ├── ProductValidation.cpp
│   │   └── README.md
│   │
│   ├── Member_04_ProductSearch/
│   │   ├── ProductDisplay.h
│   │   ├── ProductDisplay.cpp
│   │   ├── ProductSearch.h
│   │   ├── ProductSearch.cpp
│   │   └── README.md
│   │
│   ├── Member_05_CartProcessing/
│   │   ├── CartCalculator.h
│   │   ├── CartCalculator.cpp
│   │   ├── ProductSort.h
│   │   ├── ProductSort.cpp
│   │   ├── TestCases.md
│   │   └── TestReport.md
│   │
│   ├── Integration/
│   │   ├── main.cpp
│   │   ├── CartController.h
│   │   ├── CartController.cpp
│   │   └── CMakeLists.txt
│   │
│   └── tests/
│       ├── test_doubly_linked_list.cpp
│       ├── test_product_manager.cpp
│       ├── test_product_search.cpp
│       └── test_cart_calculator.cpp
│
├── Frontend/
│   ├── index.html
│   │
│   ├── pages/
│   │   ├── dashboard.html
│   │   └── cart.html
│   │
│   ├── css/
│   │   ├── main.css
│   │   ├── layout.css
│   │   ├── components.css
│   │   └── responsive.css
│   │
│   ├── js/
│   │   ├── app.js
│   │   ├── cart.js
│   │   ├── product.js
│   │   ├── search.js
│   │   ├── calculator.js
│   │   ├── api.js
│   │   └── validation.js
│   │
│   ├── components/
│   │   ├── header.html
│   │   ├── sidebar.html
│   │   ├── product-table.html
│   │   ├── product-form.html
│   │   └── cart-summary.html
│   │
│   └── assets/
│       ├── images/
│       ├── icons/
│       └── fonts/
│
└── Documents/
    ├── 01_Project_Proposal/
    │   └── Project_Description.docx
    │
    ├── 02_Analysis_Design/
    │   ├── Functional_Requirements.docx
    │   ├── Data_Structure_Design.docx
    │   └── System_Architecture.png
    │
    ├── 03_Task_Assignment/
    │   └── Team_Task_Assignment.xlsx
    │
    ├── 04_Testing/
    │   ├── Test_Cases.xlsx
    │   └── Test_Report.docx
    │
    └── 05_Final_Report/
        ├── Final_Report.docx
        └── Presentation.pptx
```

## 8. Phân công nhiệm vụ

| Thành viên | Vai trò | Nhiệm vụ |
|---|---|---|
| Nguyễn Văn Luyến (TV1) | Backend nền tảng | Thiết kế Node, xây dựng DSLK đôi, quản lý bộ nhớ và kiểm thử cấu trúc dữ liệu |
| Nguyễn Quang Nhân (TV2) | Frontend và tích hợp | Xây dựng giao diện, kiến trúc tổng thể, phương án kết nối và tích hợp |
| Vũ Nguyễn Nhã Uyên (TV3) | Quản lý sản phẩm | Thêm, xóa, cập nhật và kiểm tra dữ liệu sản phẩm |
| Trần Thị Mỹ Dung (TV4) | Tìm kiếm và hiển thị | Tìm kiếm, hiển thị, duyệt danh sách xuôi và ngược |
| Nguyễn Đinh Nhật Hà My (TV5) | Xử lý giỏ hàng | Tính tổng tiền, tổng số lượng và sắp xếp sản phẩm |

## 9. Chuỗi công việc và bàn giao

mermaid
flowchart TD
    A["TV2 - Nguyễn Quang Nhân<br/>Chuẩn bị yêu cầu, kiến trúc và quy ước"]
    B["TV1 - Nguyễn Văn Luyến<br/>Xây dựng nền tảng DSLK đôi"]
    C["TV3 - Vũ Nguyễn Nhã Uyên<br/>Thêm, xóa, cập nhật"]
    D["TV4 - Trần Thị Mỹ Dung<br/>Tìm kiếm, hiển thị, duyệt"]
    E["TV5 - Nguyễn Đinh Nhật Hà My<br/>Tính toán và sắp xếp"]
    F["TV2 - Nguyễn Quang Nhân<br/>Tích hợp Frontend và Backend"]
    G["Cả nhóm<br/>Kiểm thử và sửa lỗi"]
    H["Chốt sản phẩm<br/>22/11/2026"]

    A -->|"Bàn giao yêu cầu"| B
    B -->|"Bàn giao cấu trúc dữ liệu"| C
    B -->|"Bàn giao cấu trúc dữ liệu"| D
    B -->|"Bàn giao cấu trúc dữ liệu"| E
    C -->|"Module quản lý sản phẩm"| E
    D -->|"Module tìm kiếm, hiển thị"| E
    C -->|"Bàn giao module"| F
    D -->|"Bàn giao module"| F
    E -->|"Bàn giao module"| F
    F -->|"Phiên bản tích hợp"| G
    G --> H


### Quy tắc bàn giao

1. Người hoàn thành phải tự kiểm thử module.
2. Đẩy mã nguồn lên đúng thư mục và nhánh Git.
3. Gửi tài liệu hướng dẫn sử dụng cho người nhận.
4. Người nhận phải chạy thử và xác nhận.
5. Lỗi thuộc module nào thì người phụ trách module đó xử lý.
6. TV1 quản lý cấu trúc dữ liệu chung; TV2 quản lý giao diện và tích hợp.

## 10. Kế hoạch thời gian

**Thời gian thực hiện: 06/11/2026 – 22/11/2026.**

| Thời gian | Công việc |
|---|---|
| 06/11–08/11 | Phân tích yêu cầu, phân công và thống nhất kiến trúc |
| 09/11–13/11 | TV1 xây dựng và kiểm thử DSLK đôi |
| 14/11–17/11 | TV3 và TV4 phát triển nghiệp vụ |
| 18/11–19/11 | TV5 hoàn thiện tính toán và sắp xếp |
| 20/11 | TV2 tích hợp Frontend và Backend |
| 21/11 | Cả nhóm kiểm thử, sửa lỗi và hoàn thiện tài liệu |
| 22/11 | Chốt sản phẩm và nghiệm thu nội bộ |

## 11. Kiểm thử hệ thống

Các trường hợp cần kiểm thử:

- Danh sách rỗng.
- Thêm node đầu tiên và nhiều node.
- Xóa node đầu, cuối, giữa và node duy nhất.
- Kiểm tra ID trùng.
- Cập nhật giá và số lượng.
- Tìm kiếm sản phẩm tồn tại và không tồn tại.
- Duyệt danh sách theo hai chiều.
- Tính tổng tiền và tổng số lượng.
- Sắp xếp sản phẩm theo giá.
- Kiểm tra liên kết `prev` và `next` sau thao tác.
- Kiểm thử các luồng chức năng trên giao diện.

## 12. Quy định Git/GitHub

- Mỗi thành viên làm việc trên nhánh riêng.
- Không tự ý commit trực tiếp vào `main` nếu chưa được thống nhất.
- Tên commit phải mô tả đúng nội dung thay đổi.
- Tạo Pull Request khi hoàn thành module.
- Kiểm tra mã nguồn trước khi hợp nhất.
- Không đưa file biên dịch và file tạm không cần thiết lên repository.

### Ví dụ tên nhánh

text
feature/doubly-linked-list
feature/frontend
feature/product-manager
feature/product-search
feature/cart-processing


### Ví dụ commit

text
feat: implement doubly linked list
feat: add product management
feat: implement product search
feat: calculate cart total
fix: correct node deletion
docs: update project README


## 13. Kết quả dự kiến

- Xây dựng cấu trúc DSLK đôi bằng C++.
- Hoàn thiện các module quản lý sản phẩm.
- Tìm kiếm và hiển thị sản phẩm.
- Tính tổng số lượng và giá trị giỏ hàng.
- Sắp xếp sản phẩm theo giá.
- Xây dựng giao diện quản lý giỏ hàng.
- Tích hợp và kiểm thử các chức năng trong phạm vi dự án.
- Hoàn thiện tài liệu và báo cáo.

## 14. Thông tin dự án

| Thuộc tính | Nội dung |
|---|---|
| Tên dự án | Hệ thống quản lý giỏ hàng online |
| Tên tiếng Anh | Online Shopping Cart Management System |
| Backend | C++ |
| Frontend | HTML, CSS, JavaScript |
| Cấu trúc dữ liệu | Doubly Linked List |
| Thành viên | 5 |
| Ngày bắt đầu | 06/11/2026 |
| Hạn hoàn thành | 22/11/2026 |
| Quản lý mã nguồn | GitHub |

---

<p align="center">

**ONLINE SHOPPING CART MANAGEMENT SYSTEM**

*Developed as a team project.*

</p>
