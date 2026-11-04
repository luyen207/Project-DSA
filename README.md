# 🛒 HỆ THỐNG GIỎ HÀNG TRỰC TUYẾN

**Online Cart System** – Đồ án cuối kì học phần **Cấu trúc dữ liệu và giải thuật**

Ứng dụng web quản lý giỏ hàng có **đăng nhập và phân quyền hai vai trò (Customer, Admin)**. Lõi nghiệp vụ giỏ hàng được cài đặt bằng **danh sách liên kết đôi (Doubly Linked List)** trên backend **C++17 + Drogon**, giao diện **React + TypeScript**, lưu trữ lâu dài bằng **PostgreSQL**.

---

## 👥 Thông tin nhóm thực hiện

| Vai trò | Họ và tên |
|---|---|
| **Giảng viên hướng dẫn** | TS. Lê Văn Quốc Anh |
| **Nhóm trưởng** | Nguyễn Văn Luyến |
| Thành viên | Nguyễn Quang Nhân |
| Thành viên | Trần Thị Mỹ Dung |
| Thành viên | Nguyễn Đinh Nhật Hà My |
| Thành viên | Vũ Nguyễn Nhã Uyên |

Phân công và đóng góp chi tiết của từng thành viên: [`docs/team/phan-cong.md`](docs/team/phan-cong.md).

---

## 📑 Mục lục

1. [Giới thiệu sản phẩm](#1-giới-thiệu-sản-phẩm)
2. [Nhu cầu xây dựng và mục tiêu](#2-nhu-cầu-xây-dựng-và-mục-tiêu)
3. [Phạm vi chức năng](#3-phạm-vi-chức-năng)
4. [So sánh với các hệ thống hiện có](#4-so-sánh-với-các-hệ-thống-hiện-có)
5. [Công nghệ sử dụng](#5-công-nghệ-sử-dụng)
6. [Kiến trúc hệ thống](#6-kiến-trúc-hệ-thống)
7. [Xác thực và phân quyền](#7-xác-thực-và-phân-quyền)
8. [Thiết kế cấu trúc dữ liệu và giải thuật](#8-thiết-kế-cấu-trúc-dữ-liệu-và-giải-thuật)
9. [Thiết kế cơ sở dữ liệu](#9-thiết-kế-cơ-sở-dữ-liệu)
10. [Thiết kế REST API](#10-thiết-kế-rest-api)
11. [Luồng xử lý hệ thống](#11-luồng-xử-lý-hệ-thống)
12. [Cấu trúc thư mục tổng thể](#12-cấu-trúc-thư-mục-tổng-thể)
13. [Cài đặt và triển khai](#13-cài-đặt-và-triển-khai)
14. [Hướng dẫn sử dụng](#14-hướng-dẫn-sử-dụng)
15. [Lộ trình 4 phiên bản](#15-lộ-trình-4-phiên-bản)
16. [Kiểm thử và đảm bảo chất lượng](#16-kiểm-thử-và-đảm-bảo-chất-lượng)
17. [Quy trình phát triển](#17-quy-trình-phát-triển)
18. [Quản lý rủi ro](#18-quản-lý-rủi-ro)
19. [Khai thác và sử dụng AI](#19-khai-thác-và-sử-dụng-ai)
20. [Hướng phát triển](#20-hướng-phát-triển)

---

## 1. Giới thiệu sản phẩm

**Online Cart System** mô phỏng quy trình mua sắm trực tuyến hoàn chỉnh cùng khu vực quản trị:

```
Đăng nhập → Duyệt sản phẩm → Quản lý giỏ hàng → Áp khuyến mãi / voucher → Tạo đơn → Thanh toán → Theo dõi đơn hàng
                                                                                         ▲
                              Admin: quản lý sản phẩm · voucher · đơn hàng · người dùng · thống kê
```

Hệ thống có hai vai trò với giao diện và quyền hạn tách biệt:

| Vai trò | Mục đích | Giao diện |
|---|---|---|
| **Customer** (khách hàng) | Mua sắm: giỏ hàng, đặt hàng, thanh toán, theo dõi đơn | Giao diện cửa hàng |
| **Admin** (quản trị viên) | Vận hành: sản phẩm, voucher, đơn hàng, người dùng, thống kê | Giao diện quản trị riêng |

Điểm cốt lõi của đồ án: **toàn bộ giỏ hàng được tổ chức bằng danh sách liên kết đôi tự cài đặt** (không dùng `std::list`, `std::vector`). Mỗi mặt hàng là một node:

| Trường | Kiểu | Ý nghĩa |
|---|---|---|
| `id` | `int` | Mã mặt hàng |
| `tenHang` | `string` | Tên mặt hàng |
| `gia` | `long long` | Đơn giá (VNĐ, số nguyên) |
| `soLuong` | `int` | Số lượng |
| `truoc` | `CartNode*` | Con trỏ tới node đứng trước |
| `sau` | `CartNode*` | Con trỏ tới node đứng sau |
| `daChon` | `bool` | Mặt hàng có được chọn để thanh toán |
| `daGhim` | `bool` | Mặt hàng được ghim đầu giỏ |
| `phanTramGiam` | `int` | Phần trăm khuyến mãi áp dụng cho mặt hàng |

Ba trường đầu cùng `truoc`, `sau` là các trường bắt buộc theo đề tài; các trường còn lại phục vụ chức năng mở rộng.

---

## 2. Nhu cầu xây dựng và mục tiêu

### 2.1. Nhu cầu

- **Về học phần:** vận dụng cấu trúc dữ liệu và giải thuật vào một bài toán thực tế, có phân tích độ phức tạp, kiểm thử và đánh giá hiệu năng.
- **Về thực tiễn:** thương mại điện tử là lĩnh vực phổ biến; giỏ hàng là thành phần trung tâm, ảnh hưởng trực tiếp đến trải nghiệm và tỉ lệ hoàn tất đơn hàng. Một hệ thống bán hàng thực tế luôn có người mua và người quản trị với quyền hạn khác nhau.
- **Về kỹ thuật:** danh sách liên kết đôi phù hợp với giỏ hàng vì:
  - Chèn, xóa, di chuyển node chỉ thay đổi con trỏ, không dịch chuyển phần tử.
  - Duyệt được hai chiều, thuận lợi cho thao tác dời lên/xuống và Undo/Redo.
  - Xóa một node khi đã có con trỏ tới nó có độ phức tạp **O(1)**.

### 2.2. Mục tiêu

1. Đáp ứng đủ yêu cầu bắt buộc của đề tài: thêm, xem, sửa, xóa mặt hàng và tạo đơn hàng từ giỏ.
2. Xây dựng hệ thống chạy ổn định, kiểm soát được dữ liệu không hợp lệ và các trường hợp biên.
3. Bổ sung các chức năng thực tế: voucher, khuyến mãi, thanh toán, theo dõi đơn hàng thời gian thực.
4. Có **tài khoản, đăng nhập và phân quyền** Customer/Admin, kiểm soát truy cập ở cả backend và giao diện.
5. Có khu vực **quản trị** để vận hành sản phẩm, voucher, đơn hàng, người dùng và xem thống kê.
6. Giao diện web responsive, thao tác rõ ràng, phản hồi lỗi thân thiện.
7. Có phân tích độ phức tạp, bộ test và số liệu hiệu năng để phục vụ báo cáo và vấn đáp.

---

## 3. Phạm vi chức năng

### 3.1. Chức năng bắt buộc theo đề tài

| Mã | Chức năng | Mô tả |
|---|---|---|
| F01 | Thêm mặt hàng vào giỏ | Thêm node mới vào cuối danh sách; nếu đã có thì cộng dồn số lượng |
| F02 | Xem giỏ hàng | Duyệt danh sách, hiển thị từng dòng và tổng tiền |
| F03 | Sửa mặt hàng | Cập nhật số lượng |
| F04 | Xóa mặt hàng | Gỡ node khỏi danh sách |
| F05 | Tạo đơn hàng từ giỏ | Tạo đơn từ các mặt hàng được chọn |

### 3.2. Chức năng mở rộng cho khách hàng (Customer)

| Mã | Chức năng | Mô tả |
|---|---|---|
| F06 | Ghim mặt hàng lên đầu giỏ | Tách node khỏi vị trí cũ và gắn lên đầu danh sách |
| F07 | Dời mặt hàng lên/xuống | Đổi vị trí với node liền trước/sau |
| F08 | Chọn/bỏ chọn mặt hàng để thanh toán | Chỉ các node `daChon = true` được tính vào đơn |
| F09 | Gợi ý tiết kiệm | Thông báo số tiền cần mua thêm để đạt miễn phí vận chuyển hoặc đủ điều kiện voucher |
| F10 | Undo / Redo nhiều bước | Hoàn tác và làm lại thao tác thêm, sửa, xóa, ghim, dời, chọn |
| F11 | Áp dụng voucher | Voucher giảm theo phần trăm hoặc số tiền cố định, có điều kiện áp dụng |
| F12 | Khuyến mãi theo mặt hàng | Mỗi mặt hàng có thể có mức giảm riêng theo thời gian hiệu lực |
| F13 | Kiểm tra tồn kho | Không cho thêm hoặc sửa vượt số lượng tồn |
| F14 | Sắp xếp giỏ hàng | Sắp theo tên, giá, số lượng (tăng/giảm) |
| F15 | Tìm kiếm, lọc trong giỏ | Tìm theo tên hoặc ID, lọc theo khoảng giá |
| F16 | Thanh toán | Thanh toán khi nhận hàng (COD), cổng thanh toán giả lập, cổng VNPay môi trường sandbox |
| F17 | Theo dõi đơn hàng thời gian thực | Cập nhật trạng thái đơn qua WebSocket, hiển thị mốc thời gian từng bước |
| F18 | Hủy đơn hàng | Hủy khi đơn đang ở trạng thái chờ xác nhận |
| F23 | Lịch sử đơn hàng của tôi | Xem danh sách và chi tiết các đơn đã đặt |

### 3.3. Chức năng tài khoản và phân quyền (Customer và Admin)

| Mã | Chức năng | Mô tả |
|---|---|---|
| F19 | Đăng ký tài khoản | Khách tạo tài khoản; tài khoản mới luôn có vai trò Customer |
| F20 | Đăng nhập / đăng xuất | Xác thực bằng email và mật khẩu, nhận JWT |
| F21 | Phân quyền theo vai trò | Kiểm soát truy cập theo Customer/Admin ở backend; giao diện hiển thị theo vai trò |
| F22 | Quản lý hồ sơ và đổi mật khẩu | Xem, sửa họ tên, số điện thoại; đổi mật khẩu |

### 3.4. Chức năng quản trị (Admin)

| Mã | Chức năng | Mô tả |
|---|---|---|
| F24 | Quản lý sản phẩm | Thêm, sửa, ngừng bán sản phẩm; cập nhật tồn kho; thiết lập khuyến mãi theo sản phẩm |
| F25 | Quản lý voucher | Tạo, sửa, bật/tắt voucher; theo dõi số lượt đã dùng |
| F26 | Quản lý đơn hàng | Xem tất cả đơn, lọc theo trạng thái/ngày/mã đơn, cập nhật trạng thái, hủy đơn |
| F27 | Thống kê (Dashboard) | Số đơn, doanh thu, đơn theo trạng thái, sản phẩm bán chạy, sản phẩm sắp hết hàng |
| F28 | Quản lý người dùng | Xem danh sách tài khoản, khóa/mở khóa tài khoản Customer |

### 3.5. Ngoài phạm vi (xem [Hướng phát triển](#20-hướng-phát-triển))

Hoàn tiền, đánh giá sản phẩm, tích hợp đơn vị vận chuyển thật, gợi ý sản phẩm, quên mật khẩu qua email, đăng nhập bằng tài khoản mạng xã hội.

### 3.6. Quy ước truy cập

- Khách chưa đăng nhập **chỉ xem được danh sách và chi tiết sản phẩm**; mua hàng yêu cầu đăng nhập.
- Mỗi tài khoản Customer có **một giỏ hàng riêng** gắn với tài khoản.
- Tài khoản **Admin chỉ dùng khu vực quản trị**, không có giỏ hàng và không đặt hàng.
- Tài khoản Admin không thể tự đăng ký; được tạo khi khởi tạo hệ thống (xem [mục 13.4](#134-biến-môi-trường-env)).

---

## 4. So sánh với các hệ thống hiện có

Các chức năng phổ biến (voucher, thanh toán, theo dõi đơn, chọn từng món, trang quản trị) đã có ở các sàn thương mại điện tử lớn. Đồ án **tham khảo các chức năng này** và cài đặt lại trên cấu trúc dữ liệu tự xây dựng. Điểm khác biệt:

| Tiêu chí | Hệ thống thương mại điện tử lớn | Online Cart System |
|---|---|---|
| Mục đích | Vận hành thương mại thực tế | Mô hình hóa lõi giỏ hàng để thể hiện cấu trúc dữ liệu và giải thuật |
| Tổ chức giỏ hàng | Bảng cơ sở dữ liệu, xử lý phía server | Danh sách liên kết đôi trong bộ nhớ, đồng bộ xuống PostgreSQL |
| Undo/Redo nhiều bước cho giỏ | Thường chỉ có hoàn tác ngắn hạn sau khi xóa | Lịch sử thao tác bằng danh sách liên kết đôi, lùi/tiến nhiều bước |
| Sắp xếp và dời vị trí trong giỏ | Thường theo thời gian thêm hoặc theo cửa hàng | Người dùng chủ động sắp xếp, ghim, dời vị trí |
| Phân quyền | Nhiều tầng vai trò, hạ tầng xác thực riêng | Hai vai trò, kiểm soát truy cập tự cài đặt ở backend (JWT + filter), kiểm thử theo ma trận quyền |
| Khả năng giải thích | Hộp đen | Mọi thao tác con trỏ, luồng xác thực và độ phức tạp đều có thể trình bày và kiểm chứng |

---

## 5. Công nghệ sử dụng

### 5.1. Bộ công nghệ chính thức

| Lớp | Công nghệ | Vai trò |
|---|---|---|
| Frontend | **React + TypeScript** | Xây dựng giao diện website |
| Backend | **C++17 + Drogon** | REST API, WebSocket và nghiệp vụ giỏ hàng |
| Cơ sở dữ liệu | **PostgreSQL** | Lưu trữ dữ liệu lâu dài |
| Giao diện | **Tailwind CSS** | Thiết kế giao diện responsive |
| Quản lý mã nguồn | **Git + GitHub** | Quản lý source code và làm việc nhóm |

### 5.2. Công cụ bổ trợ

| Công cụ | Vai trò |
|---|---|
| CMake | Quản lý quá trình biên dịch C++ |
| Postman | Kiểm thử API |
| VS Code | Môi trường phát triển |
| Docker Compose | Khởi chạy backend và database đồng nhất |
| GitHub Projects | Quản lý nhiệm vụ |

### 5.3. Thư viện và công cụ đi kèm

| Công cụ | Vai trò |
|---|---|
| Vite | Công cụ build và dev server cho React + TypeScript |
| React Router | Điều hướng và bảo vệ route theo vai trò |
| jwt-cpp | Tạo và xác thực JWT trên backend |
| libsodium | Băm mật khẩu bằng Argon2id |
| GoogleTest | Kiểm thử đơn vị backend C++ |
| Vitest + React Testing Library | Kiểm thử frontend |
| ESLint + Prettier | Chuẩn hóa mã nguồn frontend |
| clang-format | Chuẩn hóa mã nguồn C++ |

### 5.4. Nguyên tắc kỹ thuật

- Tiền tệ lưu bằng **số nguyên (VNĐ)**, không dùng số thực để tránh sai số làm tròn.
- Danh sách liên kết đôi **tự cài đặt**, không dùng container có sẵn của STL cho giỏ hàng và lịch sử thao tác.
- Mọi truy vấn cơ sở dữ liệu dùng **câu lệnh tham số hóa** để chống SQL Injection.
- Mỗi giỏ hàng được bảo vệ bằng `std::mutex` vì Drogon xử lý request đa luồng.
- Mật khẩu **không bao giờ lưu hoặc ghi log ở dạng gốc**; chỉ lưu giá trị băm Argon2id.
- **Phân quyền được thực thi ở backend**; việc ẩn/hiện thành phần giao diện chỉ phục vụ trải nghiệm, không được xem là lớp bảo mật.

---

## 6. Kiến trúc hệ thống

### 6.1. Kiến trúc tổng thể

```
┌──────────────────────────────────────────────────────────────┐
│                      TRÌNH DUYỆT (Client)                    │
│   React + TypeScript + Tailwind CSS                          │
│   Giao diện Customer : Sản phẩm · Giỏ hàng · Đơn của tôi     │
│   Giao diện Admin    : Dashboard · Sản phẩm · Voucher ·      │
│                        Đơn hàng · Người dùng                 │
└───────────────┬───────────────────────────┬──────────────────┘
                │ HTTP/JSON + JWT           │ WebSocket + JWT
                ▼                           ▼
┌──────────────────────────────────────────────────────────────┐
│                  BACKEND – C++17 + Drogon                    │
│                                                              │
│  ┌──────────── Filter (chạy trước Controller) ────────────┐  │
│  │ CorsFilter → AuthFilter (xác thực) → RoleFilter (quyền)│  │
│  └───────────────────────────┬────────────────────────────┘  │
│  ┌─────────────────────  Controller  ─────────────────────┐  │
│  │ Auth · Account · Product · Cart · Order · Payment      │  │
│  │ OrderWebSocket · Admin(Product/Voucher/Order/User/Stats)│ │
│  └───────────────────────────┬────────────────────────────┘  │
│  ┌─────────────────────  Service  ────────────────────────┐  │
│  │ AuthService · JwtService · PasswordHasher · UserService│  │
│  │ CartService · PricingService · VoucherService          │  │
│  │ OrderService · PaymentService · TrackingService        │  │
│  │ StatsService                                           │  │
│  └───────────────┬────────────────────────┬───────────────┘  │
│  ┌──────  Domain (Cấu trúc dữ liệu)  ──┐   │                  │
│  │ CartList (DSLK đôi)                 │   │                  │
│  │ HistoryList (DSLK đôi – Undo/Redo)  │   │                  │
│  │ CartManager (userId → CartList)     │   │                  │
│  └─────────────────────────────────────┘   │                  │
│  ┌─────────────────  Repository  ─────────▼──────────────┐   │
│  │ UserRepo · ProductRepo · CartRepo · OrderRepo          │   │
│  │ VoucherRepo · PaymentRepo · StatsRepo                  │   │
│  └───────────────────────────┬────────────────────────────┘  │
└──────────────────────────────┼───────────────────────────────┘
                               ▼
                   ┌───────────────────────┐        ┌────────────────────┐
                   │      PostgreSQL       │        │ Cổng thanh toán    │
                   │ users · products ·    │        │ (mock / VNPay      │
                   │ carts · orders …      │        │  sandbox)          │
                   └───────────────────────┘        └────────────────────┘
```

### 6.2. Trách nhiệm từng lớp

| Lớp | Trách nhiệm | Không được làm |
|---|---|---|
| **Filter** | Xác thực JWT, kiểm tra vai trò theo chính sách của từng route, xử lý CORS | Chứa nghiệp vụ |
| **Controller** | Nhận request, kiểm tra định dạng đầu vào, trả response JSON thống nhất | Chứa nghiệp vụ |
| **Service** | Nghiệp vụ: xác thực, tính giá, áp voucher, tạo đơn, chuyển trạng thái, kiểm tra quyền sở hữu dữ liệu | Thao tác SQL trực tiếp |
| **Domain** | Cấu trúc dữ liệu `CartList`, `HistoryList`; thao tác con trỏ | Biết về HTTP hay cơ sở dữ liệu |
| **Repository** | Truy vấn PostgreSQL, ánh xạ dữ liệu | Chứa nghiệp vụ |

### 6.3. Mô hình lưu trữ giỏ hàng (in-memory + write-through)

- **Nguồn thao tác chính** là `CartList` trong bộ nhớ, do `CartManager` giữ theo `userId`.
- Mỗi thao tác thay đổi giỏ được **ghi đồng bộ (write-through)** xuống bảng `cart_items`, thứ tự node lưu ở cột `position`.
- Khi giỏ chưa có trong bộ nhớ (khởi động lại server, người dùng đăng nhập lại), `CartRepo` đọc `cart_items ORDER BY position` và **dựng lại danh sách liên kết đôi**.
- Lịch sử Undo/Redo (`HistoryList`) lưu trong bộ nhớ theo từng người dùng, không ghi xuống cơ sở dữ liệu.

### 6.4. Định dạng response thống nhất

```json
// Thành công
{ "success": true, "data": { }, "error": null }

// Thất bại
{ "success": false, "data": null,
  "error": { "code": "OUT_OF_STOCK", "message": "Số lượng vượt quá tồn kho (còn 3)." } }
```

Mã lỗi chuẩn:

| Nhóm | Mã lỗi |
|---|---|
| Chung | `VALIDATION_ERROR`, `NOT_FOUND`, `INVALID_STATE`, `INTERNAL_ERROR` |
| Xác thực, phân quyền | `UNAUTHORIZED` (HTTP 401), `TOKEN_EXPIRED` (401), `FORBIDDEN` (403), `INVALID_CREDENTIALS`, `EMAIL_EXISTS`, `ACCOUNT_LOCKED`, `WEAK_PASSWORD` |
| Giỏ hàng, đơn hàng | `OUT_OF_STOCK`, `EMPTY_SELECTION`, `NOTHING_TO_UNDO`, `NOTHING_TO_REDO` |
| Voucher | `VOUCHER_INVALID`, `VOUCHER_EXPIRED`, `MIN_ORDER_NOT_MET` |
| Thanh toán | `PAYMENT_FAILED` |

---

## 7. Xác thực và phân quyền

### 7.1. Vai trò

| Vai trò | Cách có được | Phạm vi |
|---|---|---|
| **Khách (chưa đăng nhập)** | Truy cập website | Xem sản phẩm, đăng ký, đăng nhập |
| **CUSTOMER** | Tự đăng ký | Giỏ hàng riêng, đặt hàng, thanh toán, theo dõi và xem lịch sử đơn của chính mình, quản lý hồ sơ |
| **ADMIN** | Được tạo khi khởi tạo hệ thống từ biến môi trường | Toàn bộ khu vực quản trị, quản lý hồ sơ của mình |

### 7.2. Ma trận phân quyền

| Chức năng | Khách | Customer | Admin |
|---|:---:|:---:|:---:|
| Xem sản phẩm | ✅ | ✅ | ✅ |
| Đăng ký, đăng nhập | ✅ | — | — |
| Giỏ hàng (F01–F15) | ❌ | ✅ (của mình) | ❌ |
| Tạo đơn, thanh toán (F05, F16) | ❌ | ✅ | ❌ |
| Xem, theo dõi, hủy đơn | ❌ | ✅ (đơn của mình) | ✅ (mọi đơn, qua khu quản trị) |
| Lịch sử đơn của tôi (F23) | ❌ | ✅ | ❌ |
| Hồ sơ, đổi mật khẩu (F22) | ❌ | ✅ | ✅ |
| Quản lý sản phẩm (F24) | ❌ | ❌ | ✅ |
| Quản lý voucher (F25) | ❌ | ❌ | ✅ |
| Quản lý đơn hàng (F26) | ❌ | ❌ | ✅ |
| Thống kê (F27) | ❌ | ❌ | ✅ |
| Quản lý người dùng (F28) | ❌ | ❌ | ✅ |

Quy tắc bổ sung:

- Admin không thể khóa chính mình hoặc tài khoản Admin khác.
- Customer chỉ truy cập dữ liệu của chính mình; truy cập đơn hàng của người khác trả về `NOT_FOUND` (không tiết lộ sự tồn tại của dữ liệu).

### 7.3. Cơ chế xác thực

| Hạng mục | Thiết kế |
|---|---|
| Định danh đăng nhập | Email (chuẩn hóa chữ thường, duy nhất) |
| Chính sách mật khẩu | Tối thiểu 8 ký tự, gồm cả chữ và số |
| Lưu mật khẩu | Băm **Argon2id** (libsodium), không lưu mật khẩu gốc |
| Token | **JWT** ký HS256, gồm `sub` (userId), `role`, `iat`, `exp`; thời hạn cấu hình bằng `JWT_EXPIRES_MINUTES` |
| Khóa ký | `JWT_SECRET` đọc từ biến môi trường, không đưa vào mã nguồn |
| Gửi token | Header `Authorization: Bearer <token>`; với WebSocket gửi qua tham số `?token=` khi kết nối |
| Đăng xuất | Client xóa token; server không lưu phiên (stateless), token tự hết hạn theo `exp` |
| Kiểm tra trạng thái tài khoản | `AuthFilter` kiểm tra tài khoản còn `ACTIVE` ở mỗi request, nên khóa tài khoản có hiệu lực ngay dù token chưa hết hạn |
| Thông báo lỗi đăng nhập | Dùng chung một thông báo cho "sai email" và "sai mật khẩu" để không lộ email nào đã đăng ký |
| Giới hạn đăng nhập sai | Sai quá `LOGIN_MAX_ATTEMPTS` lần liên tiếp thì khóa tạm `LOGIN_LOCK_MINUTES` phút (`ACCOUNT_LOCKED`) |

> Token lưu ở `localStorage` của trình duyệt để đơn giản hóa cài đặt. Giao diện không dùng `dangerouslySetInnerHTML` và mọi nội dung hiển thị đều được React escape để giảm rủi ro XSS. Hướng nâng cấp: refresh token trong cookie `HttpOnly` (xem [mục 20](#20-hướng-phát-triển)).

### 7.4. Cơ chế phân quyền ở backend

Mỗi request đi qua chuỗi kiểm soát trước khi tới nghiệp vụ:

```
Request
  │
  ▼
CorsFilter       : chỉ cho phép origin trong CORS_ALLOWED_ORIGINS
  │
  ▼
AuthFilter       : route công khai → bỏ qua
                   route yêu cầu đăng nhập → kiểm tra chữ ký + hạn token + trạng thái ACTIVE
                   thất bại → 401 UNAUTHORIZED / TOKEN_EXPIRED
                   thành công → gắn userId, role vào request
  │
  ▼
RoleFilter       : so vai trò với chính sách của route
                   không đủ quyền → 403 FORBIDDEN
  │
  ▼
Controller → Service : kiểm tra quyền sở hữu dữ liệu (order.user_id == userId)
  │
  ▼
Repository → PostgreSQL
```

Chính sách theo nhóm route:

| Nhóm route | Chính sách |
|---|---|
| `GET /api/products/**`, `/api/auth/register`, `/api/auth/login`, `/api/health` | Công khai |
| `/api/cart/**`, `/api/orders/**` (phía khách), `/api/payments` | Chỉ `CUSTOMER` |
| `/api/auth/me`, `/api/account/**` | `CUSTOMER` hoặc `ADMIN` |
| `/api/admin/**` | Chỉ `ADMIN` |
| `/api/payments/vnpay/return`, `/api/payments/vnpay/ipn` | Công khai nhưng bắt buộc kiểm tra chữ ký của VNPay |
| `/ws/orders/{code}` | `CUSTOMER` là chủ đơn hoặc `ADMIN` |

### 7.5. Giao diện theo vai trò

**Thanh điều hướng:**

| Mục menu | Khách | Customer | Admin |
|---|:---:|:---:|:---:|
| Sản phẩm | ✅ | ✅ | ✅ (xem trong khu quản trị) |
| Giỏ hàng (có huy hiệu số lượng) | — | ✅ | — |
| Đơn hàng của tôi | — | ✅ | — |
| Tài khoản | — | ✅ | ✅ |
| Đăng nhập / Đăng ký | ✅ | — | — |
| Dashboard | — | — | ✅ |
| Quản lý sản phẩm | — | — | ✅ |
| Quản lý voucher | — | — | ✅ |
| Quản lý đơn hàng | — | — | ✅ |
| Quản lý người dùng | — | — | ✅ |
| Đăng xuất | — | ✅ | ✅ |

**Bố cục:**

| Giao diện | Layout | Đặc điểm nhận diện |
|---|---|---|
| Customer / Khách | `CustomerLayout` | Thanh đầu trang với ô tìm kiếm, biểu tượng giỏ hàng, menu tài khoản |
| Admin | `AdminLayout` | Thanh bên (sidebar) cố định, huy hiệu "Quản trị", màu nhấn khác giao diện cửa hàng để không nhầm lẫn |

**Bảo vệ route ở frontend:**

| Route | Quyền truy cập | Khi không đủ quyền |
|---|---|---|
| `/`, `/products/:id` | Công khai | — |
| `/login`, `/register` | Chỉ khách | Đã đăng nhập → chuyển về trang chủ theo vai trò |
| `/cart`, `/checkout`, `/payment/result`, `/orders`, `/orders/:code` | `CUSTOMER` | Chưa đăng nhập → `/login?redirect=…`; là Admin → trang 403 |
| `/account` | `CUSTOMER`, `ADMIN` | Chưa đăng nhập → `/login` |
| `/admin`, `/admin/products`, `/admin/vouchers`, `/admin/orders`, `/admin/users` | `ADMIN` | Chưa đăng nhập → `/login`; là Customer → trang 403 (`ForbiddenPage`) |

**Điều hướng sau đăng nhập:** Customer về trang trước đó (tham số `redirect`) hoặc trang chủ; Admin về `/admin`.

**Xử lý phiên hết hạn:** khi API trả `TOKEN_EXPIRED`, `AuthContext` xóa token, hiển thị thông báo "Phiên đăng nhập đã hết hạn" và chuyển về `/login` kèm `redirect` để quay lại trang đang làm.

### 7.6. Luồng đăng ký và đăng nhập

```
Đăng ký
  Client ── POST /auth/register {email, password, fullName} ──► AuthController
                                                                 │ kiểm tra định dạng, độ mạnh mật khẩu
                                                                 ▼
                                                              AuthService
                                                                 │ email đã tồn tại? → EMAIL_EXISTS
                                                                 │ băm Argon2id, role = CUSTOMER
                                                                 ▼
                                                              UserRepo → INSERT users
  Client ◄── 201 {user} ───────────────────────────────────────┘

Đăng nhập
  Client ── POST /auth/login {email, password} ──► AuthService
                                                     │ tìm user theo email
                                                     │ LOCKED hoặc đang bị khóa tạm → ACCOUNT_LOCKED
                                                     │ so khớp mật khẩu với giá trị băm
                                                     │   sai → tăng failed_attempts → INVALID_CREDENTIALS
                                                     │   đúng → đặt lại failed_attempts, cập nhật last_login_at
                                                     ▼
                                                  JwtService.sign({sub, role, exp})
  Client ◄── 200 {token, expiresAt, user{id, email, fullName, role}}
     │
     ├─ lưu token vào AuthContext (và localStorage)
     ├─ gắn Authorization: Bearer vào mọi request tiếp theo
     └─ role = ADMIN ? điều hướng /admin : điều hướng trang trước đó hoặc /
```

### 7.7. Khởi tạo tài khoản Admin

- Khi khởi động, backend kiểm tra nếu **chưa có tài khoản ADMIN** thì tạo từ `ADMIN_EMAIL`, `ADMIN_PASSWORD`, `ADMIN_FULL_NAME` trong `.env`.
- Không có mật khẩu Admin mặc định trong mã nguồn hay trong dữ liệu mẫu.
- API đăng ký công khai **không nhận tham số vai trò**; vai trò luôn được gán `CUSTOMER` ở server.

---

## 8. Thiết kế cấu trúc dữ liệu và giải thuật

### 8.1. Cấu trúc dữ liệu

```cpp
struct CartNode {
    int id;
    std::string tenHang;
    long long gia;
    int soLuong;
    bool daChon;
    bool daGhim;
    int phanTramGiam;
    CartNode* truoc;
    CartNode* sau;
};

class CartList {
    CartNode* dau;     // head
    CartNode* cuoi;    // tail
    int kichThuoc;     // size
    // them, xoa, tim, ghimLenDau, doiLen, doiXuong, sapXep, duyet ...
};
```

`CartList` giữ cả `dau` và `cuoi` để thêm cuối và xóa cuối đều O(1).

```cpp
struct HistoryNode {
    LoaiThaoTac loai;          // THEM, SUA, XOA, GHIM, DOI, CHON
    SnapshotMatHang truoc;     // trạng thái trước thao tác
    SnapshotMatHang sau;       // trạng thái sau thao tác
    int viTriTruoc;            // vị trí trong giỏ trước thao tác
    HistoryNode* prev;
    HistoryNode* next;
};

class HistoryList {
    HistoryNode* dau;
    HistoryNode* cuoi;
    HistoryNode* hienTai;      // thao tác gần nhất đang có hiệu lực
};
```

Mỗi người dùng có một cặp `CartList` + `HistoryList` riêng do `CartManager` quản lý theo `userId`.

### 8.2. Giải thuật chính

| Chức năng | Giải thuật |
|---|---|
| Thêm mặt hàng | Duyệt tìm `id`; nếu có thì cộng dồn `soLuong`, nếu chưa thì gắn node mới sau `cuoi` |
| Xóa mặt hàng | Tìm node `p`; nối `p->truoc` với `p->sau`; cập nhật `dau`/`cuoi` nếu `p` là đầu/cuối/duy nhất; giải phóng `p` |
| Ghim lên đầu | Tách `p` khỏi vị trí cũ, gắn lên trước `dau`, cập nhật `dau`; xử lý riêng khi `p` đã là `dau` hoặc là `cuoi` |
| Dời lên/xuống | Đổi dữ liệu giữa `p` và node kề trước/sau; giữ nguyên liên kết con trỏ |
| Sắp xếp | Bubble sort trên danh sách, hoán đổi dữ liệu giữa hai node kề nhau |
| Undo | Lấy `hienTai`, áp dụng thao tác nghịch đảo, lùi `hienTai = hienTai->prev` |
| Redo | Tiến `hienTai = hienTai->next`, áp dụng lại thao tác |
| Thao tác mới sau Undo | Xóa toàn bộ các node phía sau `hienTai` (nhánh Redo), rồi thêm node mới |
| Tính tiền | Duyệt giỏ, cộng `thanhTien` của node `daChon`; trừ voucher; cộng phí vận chuyển |

**Công thức tính tiền** (mọi giá trị là số nguyên VNĐ):

```
giaSauGiam(node) = gia × (100 − phanTramGiam) / 100
thanhTien(node)  = giaSauGiam(node) × soLuong
tamTinh          = Σ thanhTien(node)   với node.daChon = true
giamVoucher      = theo loại voucher, không vượt giá trị giảm tối đa và không vượt tamTinh
phiVanChuyen     = 0 nếu (tamTinh − giamVoucher) ≥ ngưỡng miễn phí, ngược lại = phí mặc định
tongThanhToan    = tamTinh − giamVoucher + phiVanChuyen
```

Ngưỡng miễn phí và phí mặc định cấu hình trong `backend/config/config.json`.

### 8.3. Độ phức tạp

| Thao tác | Thời gian | Ghi chú |
|---|---|---|
| Thêm cuối (đã biết `cuoi`) | O(1) | |
| Thêm mặt hàng có kiểm tra trùng `id` | O(n) | Phải duyệt để tìm |
| Tìm theo `id` | O(n) | Duyệt tuần tự |
| Sửa số lượng | O(n) | Tìm node + gán giá trị |
| Xóa mặt hàng | O(n) tìm + O(1) gỡ node | Nhờ có `truoc`, `sau` |
| Ghim lên đầu | O(n) tìm + O(1) tách/gắn | |
| Dời lên/xuống 1 vị trí | O(n) tìm + O(1) đổi | |
| Sắp xếp (bubble sort) | O(n²) | Chấp nhận được vì giỏ hàng có kích thước nhỏ |
| Tính tổng tiền | O(n) | |
| Undo / Redo một bước | O(1) thao tác lịch sử + chi phí thao tác nghịch đảo | |
| Ghi xuống cơ sở dữ liệu sau mỗi thao tác | O(n) | Ghi lại toàn bộ `position` trong một transaction |

---

## 9. Thiết kế cơ sở dữ liệu

```
users ──1:1── carts ──< cart_items >── products
  │                                        │
  └──< orders ──< order_items >────────────┘
          │
          ├──< order_status_history
          ├──< payments
          └──> vouchers (qua voucher_code)
```

| Bảng | Cột chính |
|---|---|
| `users` | `id`, `email` (duy nhất), `password_hash`, `full_name`, `phone`, `role` (`CUSTOMER`/`ADMIN`), `status` (`ACTIVE`/`LOCKED`), `failed_attempts`, `locked_until`, `created_at`, `last_login_at` |
| `products` | `id`, `name`, `price`, `stock`, `category`, `image_url`, `active`, `promo_percent`, `promo_start`, `promo_end` |
| `carts` | `id`, `user_id` (duy nhất, khóa ngoại tới `users`), `created_at`, `updated_at` |
| `cart_items` | `cart_id`, `product_id`, `position`, `quantity`, `selected`, `pinned` |
| `vouchers` | `code`, `type` (`PERCENT`/`FIXED`), `value`, `max_discount`, `min_order`, `usage_limit`, `used_count`, `starts_at`, `expires_at`, `active` |
| `orders` | `id`, `code`, `user_id`, `subtotal`, `voucher_code`, `voucher_discount`, `shipping_fee`, `total`, `payment_method`, `payment_status`, `status`, `created_at` |
| `order_items` | `order_id`, `product_id`, `name`, `unit_price`, `discount_percent`, `quantity`, `line_total` |
| `order_status_history` | `order_id`, `status`, `changed_at`, `changed_by` |
| `payments` | `id`, `order_id`, `method`, `amount`, `provider_txn_ref`, `status`, `created_at` |

Quy tắc dữ liệu:

- `order_items` lưu **bản chụp (snapshot)** tên và giá tại thời điểm đặt, để đơn cũ không đổi khi sản phẩm đổi giá.
- Sản phẩm đã có trong đơn **không bị xóa vật lý**; Admin chọn "ngừng bán" (`active = false`), sản phẩm đó ẩn khỏi cửa hàng nhưng đơn cũ vẫn hiển thị đúng.
- Ràng buộc: `quantity > 0`, `price >= 0`, `stock >= 0`, khóa duy nhất `(cart_id, product_id)`, `users.role` và `users.status` chỉ nhận giá trị hợp lệ (`CHECK`).
- Tạo đơn, trừ tồn kho và tăng `used_count` của voucher chạy trong **một transaction**.
- Trạng thái đơn: `PENDING` → `CONFIRMED` → `PACKING` → `SHIPPING` → `COMPLETED`; nhánh `CANCELLED` chỉ đi từ `PENDING`. Chỉ cho phép chuyển sang trạng thái kế tiếp hợp lệ (máy trạng thái).
- Trạng thái thanh toán: `UNPAID`, `PENDING`, `PAID`, `FAILED`.
- `changed_by` ghi `userId` của Admin khi cập nhật thủ công, hoặc rỗng khi hệ thống tự chuyển.

---

## 10. Thiết kế REST API

Tiền tố `/api`. Cột **Quyền**: `Công khai`, `Customer`, `Admin`, `Đã đăng nhập` (Customer hoặc Admin). Route yêu cầu đăng nhập cần header `Authorization: Bearer <token>`.

### 10.1. Xác thực và tài khoản

| Method | Endpoint | Quyền | Chức năng |
|---|---|---|---|
| POST | `/api/auth/register` | Công khai | Đăng ký tài khoản Customer – F19 |
| POST | `/api/auth/login` | Công khai | Đăng nhập, trả JWT – F20 |
| GET | `/api/auth/me` | Đã đăng nhập | Thông tin tài khoản hiện tại |
| GET | `/api/account/profile` | Đã đăng nhập | Xem hồ sơ – F22 |
| PUT | `/api/account/profile` | Đã đăng nhập | Sửa họ tên, số điện thoại – F22 |
| PUT | `/api/account/password` | Đã đăng nhập | Đổi mật khẩu (yêu cầu mật khẩu cũ) – F22 |

### 10.2. Sản phẩm (phía cửa hàng)

| Method | Endpoint | Quyền | Mô tả |
|---|---|---|---|
| GET | `/api/products` | Công khai | Danh sách sản phẩm đang bán (hỗ trợ `?q=`, `?category=`) |
| GET | `/api/products/{id}` | Công khai | Chi tiết sản phẩm |

### 10.3. Giỏ hàng

Giỏ hàng xác định theo tài khoản trong token, không truyền mã giỏ.

| Method | Endpoint | Quyền | Chức năng |
|---|---|---|---|
| GET | `/api/cart` | Customer | Xem giỏ (hỗ trợ `?sort=`, `?order=`, `?q=`, `?minPrice=`, `?maxPrice=`) – F02, F14, F15 |
| POST | `/api/cart/items` | Customer | Thêm mặt hàng – F01 |
| PUT | `/api/cart/items/{productId}` | Customer | Sửa số lượng – F03 |
| DELETE | `/api/cart/items/{productId}` | Customer | Xóa mặt hàng – F04 |
| PATCH | `/api/cart/items/{productId}/pin` | Customer | Ghim/bỏ ghim – F06 |
| PATCH | `/api/cart/items/{productId}/move` | Customer | Dời `{ "direction": "up" \| "down" }` – F07 |
| PATCH | `/api/cart/items/{productId}/select` | Customer | Chọn/bỏ chọn – F08 |
| PATCH | `/api/cart/select-all` | Customer | Chọn/bỏ chọn tất cả |
| GET | `/api/cart/summary` | Customer | Tạm tính, giảm giá, phí vận chuyển, tổng, gợi ý tiết kiệm – F09 |
| POST | `/api/cart/undo` | Customer | Hoàn tác – F10 |
| POST | `/api/cart/redo` | Customer | Làm lại – F10 |
| POST | `/api/cart/voucher` | Customer | Áp voucher – F11 |
| DELETE | `/api/cart/voucher` | Customer | Gỡ voucher |

### 10.4. Đơn hàng và thanh toán

| Method | Endpoint | Quyền | Chức năng |
|---|---|---|---|
| POST | `/api/orders` | Customer | Tạo đơn từ các mặt hàng đã chọn – F05 |
| GET | `/api/orders` | Customer | Danh sách đơn của tôi – F23 |
| GET | `/api/orders/{code}` | Customer (chủ đơn) | Chi tiết đơn và lịch sử trạng thái – F17 |
| POST | `/api/orders/{code}/cancel` | Customer (chủ đơn) | Hủy đơn đang chờ xác nhận – F18 |
| POST | `/api/payments` | Customer (chủ đơn) | Khởi tạo thanh toán `{ "orderCode", "method" }` – F16 |
| GET | `/api/payments/vnpay/return` | Công khai | URL trình duyệt quay về sau thanh toán VNPay sandbox |
| GET | `/api/payments/vnpay/ipn` | Công khai (kiểm tra chữ ký) | URL VNPay gọi về để xác nhận kết quả |
| WS | `/ws/orders/{code}?token=` | Customer (chủ đơn) hoặc Admin | Đẩy trạng thái đơn theo thời gian thực – F17 |

### 10.5. Quản trị (chỉ Admin)

| Method | Endpoint | Chức năng |
|---|---|---|
| GET | `/api/admin/products` | Danh sách sản phẩm gồm cả sản phẩm ngừng bán (`?q=`, `?active=`) – F24 |
| POST | `/api/admin/products` | Thêm sản phẩm – F24 |
| PUT | `/api/admin/products/{id}` | Sửa thông tin, giá, tồn kho – F24 |
| PATCH | `/api/admin/products/{id}/active` | Ngừng bán / bán lại – F24 |
| PATCH | `/api/admin/products/{id}/promotion` | Đặt khuyến mãi `{ percent, start, end }` – F24, F12 |
| GET | `/api/admin/vouchers` | Danh sách voucher – F25 |
| POST | `/api/admin/vouchers` | Tạo voucher – F25 |
| PUT | `/api/admin/vouchers/{code}` | Sửa voucher – F25 |
| PATCH | `/api/admin/vouchers/{code}/active` | Bật/tắt voucher – F25 |
| GET | `/api/admin/orders` | Tất cả đơn (`?status=`, `?q=`, `?from=`, `?to=`) – F26 |
| GET | `/api/admin/orders/{code}` | Chi tiết đơn bất kỳ – F26 |
| PATCH | `/api/admin/orders/{code}/status` | Chuyển sang trạng thái kế tiếp hợp lệ – F26 |
| POST | `/api/admin/orders/{code}/cancel` | Hủy đơn đang chờ xác nhận – F26 |
| GET | `/api/admin/stats/overview` | Tổng đơn, doanh thu, đơn theo trạng thái, sản phẩm bán chạy, sản phẩm sắp hết hàng – F27 |
| GET | `/api/admin/stats/revenue` | Doanh thu theo khoảng ngày (`?from=&to=`) – F27 |
| GET | `/api/admin/users` | Danh sách tài khoản (`?q=`, `?role=`, `?status=`) – F28 |
| PATCH | `/api/admin/users/{id}/status` | Khóa/mở khóa tài khoản Customer `{ "status": "LOCKED" \| "ACTIVE" }` – F28 |

### 10.6. Hệ thống

| Method | Endpoint | Quyền | Mô tả |
|---|---|---|---|
| GET | `/api/health` | Công khai | Kiểm tra backend và kết nối cơ sở dữ liệu |

---

## 11. Luồng xử lý hệ thống

### 11.1. Luồng thêm mặt hàng vào giỏ

```
Client            Filter        CartController   CartService      CartList          DB
  │ POST /cart/items  │               │               │               │              │
  ├──────────────────►│ kiểm tra JWT + role CUSTOMER  │               │              │
  │                   ├──────────────►│ kiểm tra đầu vào              │              │
  │                   │               ├──────────────►│ kiểm tra tồn kho             │
  │                   │               │               ├─────────────────────────────►│
  │                   │               │               │◄─────────────────────────────┤
  │                   │               │               ├──────────────►│ tìm id       │
  │                   │               │               │  có  → cộng dồn              │
  │                   │               │               │  chưa → gắn cuối             │
  │                   │               │               ├─ push HistoryList            │
  │                   │               │               ├─────────────────────────────►│ ghi cart_items
  │◄──────────────────┴───────────────┴───────────────┤ 201 + giỏ mới                │
```

### 11.2. Luồng Undo/Redo

```
Thao tác 1 ⇄ Thao tác 2 ⇄ Thao tác 3 ⇄ Thao tác 4
                              ▲
                          hienTai

Undo : áp dụng nghịch đảo thao tác 3, hienTai lùi về thao tác 2
Redo : hienTai tiến lên thao tác 3, áp dụng lại thao tác 3
Thao tác mới khi hienTai ở giữa: xóa nhánh phía sau hienTai rồi thêm node mới
```

### 11.3. Luồng thanh toán và tạo đơn

```
1. Customer chọn mặt hàng (F08) → xem tóm tắt giá (F09) → áp voucher (F11)
2. Gửi POST /orders
      ├─ Không có mặt hàng được chọn  → lỗi EMPTY_SELECTION
      ├─ Kiểm tra lại tồn kho, voucher, giá tại thời điểm đặt
      └─ Transaction: tạo orders (gắn user_id) + order_items, trừ tồn kho,
                      tăng used_count voucher, xóa các mặt hàng đã đặt khỏi CartList và cart_items
3. Chọn phương thức thanh toán → POST /payments (kiểm tra chủ đơn)
      ├─ COD      : payment_status = UNPAID, đơn vào luồng xử lý ngay
      ├─ Mock     : trả kết quả thành công/thất bại giả lập
      └─ VNPay    : tạo URL kèm chữ ký → chuyển hướng người dùng
                    → VNPay gọi /vnpay/ipn → kiểm tra chữ ký, đối chiếu số tiền
                    → payment_status = PAID, đơn tiếp tục xử lý
4. Mở trang theo dõi đơn → kết nối WebSocket /ws/orders/{code}?token=
```

Callback thanh toán được xử lý **idempotent**: nhận cùng một giao dịch nhiều lần không làm thay đổi kết quả.

### 11.4. Luồng xử lý và theo dõi đơn hàng thời gian thực

Cách chuyển trạng thái đơn được chọn bằng `ORDER_FLOW_MODE`:

| Chế độ | Cách hoạt động | Dùng khi |
|---|---|---|
| `auto` | `TrackingService` tự chuyển bước theo khoảng thời gian cấu hình | Demo nhanh, không cần Admin thao tác |
| `manual` | Admin bấm cập nhật trạng thái ở trang quản lý đơn | Vận hành thực tế, demo luồng Admin |

```
Điểm chuyển trạng thái (auto: bộ đếm thời gian | manual: Admin PATCH /status)
   │  kiểm tra máy trạng thái: chỉ đi tới bước kế tiếp hợp lệ
   ├─ cập nhật orders.status + ghi order_status_history (changed_by)
   └─ đẩy sự kiện qua WebSocket tới Customer chủ đơn và các Admin đang theo dõi

PENDING ─► CONFIRMED ─► PACKING ─► SHIPPING ─► COMPLETED
   └──► CANCELLED (chỉ từ PENDING)
```

Nếu mất kết nối WebSocket, client tự kết nối lại và tải lại trạng thái hiện tại qua `GET /api/orders/{code}`.

### 11.5. Luồng điều hướng giao diện

```
                        ┌────────────── Khách ──────────────┐
                        │  /  (Sản phẩm)  ·  /login  ·  /register
                        └───────────────┬───────────────────┘
                                        │ đăng nhập
                     ┌──────────────────┴──────────────────┐
                     ▼ role = CUSTOMER                     ▼ role = ADMIN
   / → /cart → /checkout → /orders/:code             /admin (Dashboard)
        │                       ▲                      ├─ /admin/products
        └──── /orders (lịch sử) ┘                      ├─ /admin/vouchers
        /account                                       ├─ /admin/orders
                                                       ├─ /admin/users
                                                       └─ /account
```

---

## 12. Cấu trúc thư mục tổng thể

Đây là cấu trúc của phiên bản hoàn chỉnh **v4.0**. Cấu trúc theo từng phiên bản xem tại [mục 15](#15-lộ-trình-4-phiên-bản).

```
online-cart-system/
├── README.md
├── LICENSE
├── .gitignore
├── .editorconfig
├── .env.example
├── docker-compose.yml
│
├── backend/                                # C++17 + Drogon
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── config/
│   │   └── config.json                     # cổng, DB, phí vận chuyển, thời gian đổi trạng thái đơn
│   ├── src/
│   │   ├── main.cc                         # khởi tạo ứng dụng, nạp cấu hình, tạo Admin ban đầu
│   │   ├── domain/                         # cấu trúc dữ liệu (không phụ thuộc HTTP/DB)
│   │   │   ├── CartNode.h
│   │   │   ├── CartList.h
│   │   │   ├── CartList.cc                 # thêm, xóa, ghim, dời, sắp xếp, tìm, duyệt
│   │   │   ├── HistoryNode.h
│   │   │   ├── HistoryList.h
│   │   │   ├── HistoryList.cc              # Undo/Redo
│   │   │   ├── CartManager.h/.cc           # userId → CartList + HistoryList, khóa mutex
│   │   │   └── Role.h                      # enum Role { CUSTOMER, ADMIN }, UserStatus
│   │   ├── controllers/
│   │   │   ├── AuthController.h/.cc        # register, login, me
│   │   │   ├── AccountController.h/.cc     # hồ sơ, đổi mật khẩu
│   │   │   ├── ProductController.h/.cc
│   │   │   ├── CartController.h/.cc
│   │   │   ├── OrderController.h/.cc
│   │   │   ├── PaymentController.h/.cc
│   │   │   ├── OrderWebSocketController.h/.cc
│   │   │   ├── HealthController.h/.cc
│   │   │   └── admin/
│   │   │       ├── AdminProductController.h/.cc
│   │   │       ├── AdminVoucherController.h/.cc
│   │   │       ├── AdminOrderController.h/.cc
│   │   │       ├── AdminUserController.h/.cc
│   │   │       └── AdminStatsController.h/.cc
│   │   ├── services/
│   │   │   ├── AuthService.h/.cc           # đăng ký, đăng nhập, khóa tạm khi sai nhiều lần
│   │   │   ├── JwtService.h/.cc            # tạo, xác thực JWT
│   │   │   ├── PasswordHasher.h/.cc        # Argon2id (libsodium)
│   │   │   ├── UserService.h/.cc           # hồ sơ, khóa/mở khóa tài khoản
│   │   │   ├── CartService.h/.cc
│   │   │   ├── PricingService.h/.cc        # công thức tính tiền, gợi ý tiết kiệm
│   │   │   ├── VoucherService.h/.cc
│   │   │   ├── OrderService.h/.cc          # tạo đơn, máy trạng thái, kiểm tra chủ đơn
│   │   │   ├── PaymentService.h/.cc
│   │   │   ├── IPaymentGateway.h           # giao diện chung cho các cổng thanh toán
│   │   │   ├── VnpayGateway.h/.cc          # tạo URL, kiểm tra chữ ký (sandbox)
│   │   │   ├── MockGateway.h/.cc
│   │   │   ├── TrackingService.h/.cc       # đổi trạng thái đơn (auto/manual), đẩy WebSocket
│   │   │   └── StatsService.h/.cc          # thống kê cho Admin
│   │   ├── repositories/
│   │   │   ├── UserRepo.h/.cc
│   │   │   ├── ProductRepo.h/.cc
│   │   │   ├── CartRepo.h/.cc
│   │   │   ├── OrderRepo.h/.cc
│   │   │   ├── VoucherRepo.h/.cc
│   │   │   ├── PaymentRepo.h/.cc
│   │   │   └── StatsRepo.h/.cc
│   │   ├── filters/
│   │   │   ├── CorsFilter.h/.cc
│   │   │   ├── AuthFilter.h/.cc            # xác thực JWT, kiểm tra tài khoản ACTIVE
│   │   │   └── RoleFilter.h/.cc            # kiểm tra vai trò theo chính sách route
│   │   ├── dto/                            # đối tượng request/response
│   │   │   ├── AuthDto.h
│   │   │   ├── CartDto.h
│   │   │   ├── OrderDto.h
│   │   │   ├── AdminDto.h
│   │   │   └── ApiResponse.h
│   │   └── utils/
│   │       ├── Money.h                     # định dạng và tính toán tiền (số nguyên)
│   │       ├── Validator.h/.cc             # kiểm tra đầu vào, độ mạnh mật khẩu, email
│   │       ├── Time.h
│   │       └── Logger.h                    # không ghi mật khẩu, token
│   └── tests/
│       ├── CMakeLists.txt
│       ├── unit/
│       │   ├── CartListTest.cc
│       │   ├── HistoryListTest.cc
│       │   ├── PricingServiceTest.cc
│       │   ├── VoucherServiceTest.cc
│       │   ├── ValidatorTest.cc
│       │   ├── PasswordHasherTest.cc
│       │   ├── JwtServiceTest.cc           # hợp lệ, sai chữ ký, hết hạn, sửa nội dung
│       │   └── OrderStateMachineTest.cc
│       ├── integration/
│       │   ├── AuthApiTest.cc              # đăng ký, đăng nhập, khóa tạm
│       │   ├── RoleAccessTest.cc           # ma trận endpoint × vai trò
│       │   ├── OwnershipTest.cc            # Customer A không truy cập đơn của Customer B
│       │   ├── CartApiTest.cc
│       │   ├── OrderFlowTest.cc
│       │   ├── PaymentFlowTest.cc
│       │   ├── TrackingTest.cc
│       │   ├── VnpayCallbackTest.cc
│       │   ├── CancelOrderTest.cc
│       │   └── AdminApiTest.cc
│       └── performance/
│           └── CartListBenchmark.cc        # đo thời gian với n = 10, 100, 1000, 10000
│
├── frontend/                               # React + TypeScript + Tailwind CSS
│   ├── package.json
│   ├── tsconfig.json
│   ├── vite.config.ts
│   ├── tailwind.config.js
│   ├── postcss.config.js
│   ├── index.html
│   ├── Dockerfile
│   ├── public/
│   └── src/
│       ├── main.tsx
│       ├── App.tsx
│       ├── routes.tsx                      # khai báo route và quyền truy cập
│       ├── api/                            # gọi REST và WebSocket
│       │   ├── client.ts                   # gắn Authorization, xử lý 401/403
│       │   ├── authApi.ts
│       │   ├── accountApi.ts
│       │   ├── cartApi.ts
│       │   ├── orderApi.ts
│       │   ├── paymentApi.ts
│       │   ├── orderSocket.ts
│       │   └── adminApi.ts
│       ├── types/
│       │   ├── auth.ts                     # User, Role, LoginResponse
│       │   ├── cart.ts
│       │   ├── order.ts
│       │   ├── admin.ts
│       │   └── api.ts
│       ├── context/
│       │   ├── AuthContext.tsx             # user, token, login, logout, xử lý hết hạn phiên
│       │   └── CartContext.tsx
│       ├── hooks/
│       │   ├── useAuth.ts
│       │   ├── useCart.ts
│       │   ├── useOrderTracking.ts         # WebSocket + tự kết nối lại
│       │   └── useToast.ts
│       ├── components/
│       │   ├── layout/
│       │   │   ├── CustomerLayout.tsx      # header cửa hàng
│       │   │   ├── AdminLayout.tsx         # sidebar quản trị
│       │   │   ├── Header.tsx
│       │   │   ├── AdminSidebar.tsx
│       │   │   └── Footer.tsx
│       │   ├── auth/                       # ProtectedRoute, AdminRoute, GuestRoute,
│       │   │                               # LoginForm, RegisterForm, PasswordInput
│       │   ├── product/                    # ProductCard, ProductGrid
│       │   ├── cart/                       # CartItemRow, CartToolbar, CartSummary,
│       │   │                               # SavingHint, UndoRedoBar, VoucherInput
│       │   ├── checkout/                   # PaymentMethodPicker, OrderReview
│       │   ├── order/                      # OrderTimeline, OrderStatusBadge
│       │   ├── admin/                      # DataTable, StatCard, BarChart (SVG tự vẽ),
│       │   │                               # ProductForm, VoucherForm, OrderStatusControl, ConfirmDialog
│       │   └── common/                     # Button, Modal, Spinner, EmptyState, ErrorBanner
│       ├── pages/
│       │   ├── LoginPage.tsx
│       │   ├── RegisterPage.tsx
│       │   ├── ProductsPage.tsx
│       │   ├── CartPage.tsx
│       │   ├── CheckoutPage.tsx
│       │   ├── PaymentResultPage.tsx
│       │   ├── MyOrdersPage.tsx
│       │   ├── OrderTrackingPage.tsx
│       │   ├── AccountPage.tsx
│       │   ├── ForbiddenPage.tsx           # 403
│       │   ├── NotFoundPage.tsx            # 404
│       │   └── admin/
│       │       ├── AdminDashboardPage.tsx
│       │       ├── AdminProductsPage.tsx
│       │       ├── AdminVouchersPage.tsx
│       │       ├── AdminOrdersPage.tsx
│       │       └── AdminUsersPage.tsx
│       ├── utils/
│       │   ├── formatMoney.ts
│       │   ├── formatDate.ts
│       │   └── tokenStorage.ts
│       ├── styles/
│       │   └── index.css
│       └── __tests__/                      # kiểm thử route guard, form đăng nhập, giỏ hàng
│
├── database/
│   ├── migrations/
│   │   ├── 001_create_users.sql
│   │   ├── 002_create_products.sql
│   │   ├── 003_create_carts.sql
│   │   ├── 004_create_orders.sql
│   │   ├── 005_create_vouchers.sql
│   │   ├── 006_create_payments.sql
│   │   ├── 007_add_product_promotion.sql
│   │   ├── 008_create_order_status_history.sql
│   │   └── 009_add_login_lockout.sql
│   ├── seed/
│   │   ├── products.sql
│   │   └── vouchers.sql
│   └── schema.sql                          # lược đồ đầy đủ để dựng nhanh
│
├── api/
│   └── postman/
│       ├── online-cart.postman_collection.json   # chia thư mục: Auth · Customer · Admin
│       └── local.postman_environment.json        # biến tokenCustomer, tokenAdmin
│
├── docs/
│   ├── requirements.md                     # phân tích yêu cầu, trường hợp đặc biệt
│   ├── architecture.md                     # kiến trúc, sơ đồ lớp, sơ đồ tuần tự
│   ├── auth-and-roles.md                   # luồng xác thực, ma trận phân quyền, mô hình đe dọa
│   ├── data-structures.md                  # DSLK đôi, thuật toán, độ phức tạp
│   ├── database.md                         # ERD, từ điển dữ liệu
│   ├── api.md                              # đặc tả API
│   ├── test-plan.md                        # kế hoạch và bộ test case
│   ├── performance-report.md               # số liệu đo hiệu năng
│   ├── ui-guidelines.md                    # màu sắc, bố cục Customer/Admin, trạng thái giao diện
│   ├── report/
│   │   └── bao-cao-do-an.docx              # báo cáo nộp giảng viên
│   ├── demo/
│   │   └── kich-ban-demo.md                # kịch bản demo (Customer, Admin) và câu hỏi vấn đáp
│   └── team/
│       └── phan-cong.md                    # phân công và đóng góp từng thành viên
│
├── scripts/
│   ├── setup.sh                            # kiểm tra môi trường, tạo .env, sinh JWT_SECRET ngẫu nhiên
│   ├── dev.sh                              # chạy chế độ phát triển
│   ├── test-all.sh                         # chạy toàn bộ kiểm thử
│   └── seed-demo-data.sh                   # tạo tài khoản Customer mẫu và đơn hàng mẫu
│
└── .github/
    ├── workflows/
    │   ├── backend-ci.yml                  # biên dịch + chạy GoogleTest
    │   └── frontend-ci.yml                 # lint + build + Vitest
    ├── ISSUE_TEMPLATE/
    │   ├── bug_report.md
    │   └── feature_request.md
    └── pull_request_template.md
```

---

## 13. Cài đặt và triển khai

### 13.1. Yêu cầu hệ thống

| Thành phần | Phiên bản khuyến nghị |
|---|---|
| Docker + Docker Compose | Bản mới ổn định |
| Git | 2.30 trở lên |
| *(Chạy thủ công)* Trình biên dịch C++17 | GCC 9+ / Clang 10+ |
| *(Chạy thủ công)* CMake | 3.16 trở lên |
| *(Chạy thủ công)* Drogon và phụ thuộc | libpq, jsoncpp, OpenSSL, zlib, uuid, libsodium, jwt-cpp |
| *(Chạy thủ công)* Node.js | 18 LTS trở lên |
| *(Chạy thủ công)* PostgreSQL | 14 trở lên |

### 13.2. Cách 1 – Docker Compose (khuyến nghị)

Cách này cho môi trường đồng nhất giữa các thành viên, không cần cài Drogon thủ công.

```bash
# 1. Tải mã nguồn
git clone https://github.com/<ten-nhom>/online-cart-system.git
cd online-cart-system

# 2. Tạo file cấu hình môi trường (script sinh JWT_SECRET ngẫu nhiên)
./scripts/setup.sh
#    hoặc thủ công: cp .env.example .env rồi điền JWT_SECRET, ADMIN_EMAIL, ADMIN_PASSWORD

# 3. Khởi chạy toàn bộ hệ thống (database + backend + frontend)
docker compose up --build

# 4. (Tùy chọn) Nạp tài khoản Customer mẫu và đơn hàng mẫu
./scripts/seed-demo-data.sh
```

| Dịch vụ | Địa chỉ |
|---|---|
| Frontend (Customer) | http://localhost:5173 |
| Khu quản trị (Admin) | http://localhost:5173/admin |
| Backend API | http://localhost:8080/api |
| Kiểm tra sức khỏe | http://localhost:8080/api/health |
| PostgreSQL | `localhost:5432` |

Lược đồ và dữ liệu mẫu được nạp tự động khi container database khởi tạo lần đầu. Tài khoản Admin được backend tạo ở lần khởi động đầu tiên từ biến môi trường. Dừng hệ thống: `docker compose down` (thêm `-v` để xóa dữ liệu).

### 13.3. Cách 2 – Chạy thủ công

```bash
# Database
createdb online_cart
psql -d online_cart -f database/schema.sql
psql -d online_cart -f database/seed/products.sql
psql -d online_cart -f database/seed/vouchers.sql

# Backend (đọc biến môi trường từ .env)
cmake -S backend -B backend/build -DCMAKE_BUILD_TYPE=Release
cmake --build backend/build -j
./backend/build/online_cart_server          # đọc backend/config/config.json

# Frontend (cửa sổ terminal khác)
cd frontend
npm install
npm run dev
```

### 13.4. Biến môi trường (`.env`)

| Biến | Ý nghĩa | Ví dụ |
|---|---|---|
| `POSTGRES_DB` | Tên database | `online_cart` |
| `POSTGRES_USER` | Tài khoản database | `cart_user` |
| `POSTGRES_PASSWORD` | Mật khẩu database | *(tự đặt)* |
| `BACKEND_PORT` | Cổng backend | `8080` |
| `FRONTEND_PORT` | Cổng frontend | `5173` |
| `VITE_API_BASE_URL` | Địa chỉ API cho frontend | `http://localhost:8080/api` |
| `CORS_ALLOWED_ORIGINS` | Danh sách origin được phép gọi API | `http://localhost:5173` |
| `JWT_SECRET` | Khóa ký JWT (chuỗi ngẫu nhiên, tối thiểu 32 ký tự) | *(sinh bởi `setup.sh`)* |
| `JWT_EXPIRES_MINUTES` | Thời hạn token | `120` |
| `LOGIN_MAX_ATTEMPTS` | Số lần đăng nhập sai tối đa trước khi khóa tạm | `5` |
| `LOGIN_LOCK_MINUTES` | Thời gian khóa tạm | `15` |
| `ADMIN_EMAIL` | Email tài khoản Admin ban đầu | *(tự đặt)* |
| `ADMIN_PASSWORD` | Mật khẩu tài khoản Admin ban đầu | *(tự đặt, đáp ứng chính sách mật khẩu)* |
| `ADMIN_FULL_NAME` | Họ tên hiển thị của Admin | `Quản trị viên` |
| `ORDER_FLOW_MODE` | Cách chuyển trạng thái đơn: `auto` hoặc `manual` | `auto` |
| `ORDER_STEP_SECONDS` | Giây giữa các bước khi `auto` (chế độ demo) | `15` |
| `VNPAY_TMN_CODE` | Mã website trên VNPay sandbox | *(tài khoản sandbox)* |
| `VNPAY_HASH_SECRET` | Khóa tạo chữ ký VNPay sandbox | *(tài khoản sandbox)* |
| `VNPAY_RETURN_URL` | URL quay về sau thanh toán | `http://localhost:5173/payment/result` |

> Không đưa file `.env` lên GitHub. Chỉ commit `.env.example` (không chứa giá trị thật của `JWT_SECRET`, mật khẩu Admin, khóa VNPay).

### 13.5. Chạy kiểm thử

```bash
./scripts/test-all.sh                       # tất cả
ctest --test-dir backend/build              # backend (GoogleTest)
cd frontend && npm run test                 # frontend (Vitest)
```

---

## 14. Hướng dẫn sử dụng

### 14.1. Dành cho khách hàng (Customer)

1. **Đăng ký / Đăng nhập:** tại `/register` nhập họ tên, email, mật khẩu (tối thiểu 8 ký tự, có chữ và số); sau đó đăng nhập tại `/login`. Khách chưa đăng nhập vẫn xem được sản phẩm nhưng bấm **Thêm vào giỏ** sẽ được chuyển tới trang đăng nhập.
2. **Trang Sản phẩm:** chọn sản phẩm, bấm **Thêm vào giỏ**.
3. **Trang Giỏ hàng:**
   - Chỉnh số lượng, xóa mặt hàng.
   - Dùng **Ghim** để đưa mặt hàng lên đầu, **▲ ▼** để dời vị trí.
   - Tick/bỏ tick từng mặt hàng để chọn món cần thanh toán.
   - Dùng ô **tìm kiếm**, **bộ lọc giá** và **sắp xếp** khi giỏ có nhiều mặt hàng.
   - Dùng **Hoàn tác / Làm lại** để quay lại hoặc thực hiện lại thao tác.
   - Xem **gợi ý tiết kiệm** (ví dụ: *"Mua thêm 45.000đ để được miễn phí vận chuyển"*).
   - Nhập **mã voucher** và bấm **Áp dụng**.
4. **Trang Thanh toán:** kiểm tra đơn, chọn phương thức (COD, thanh toán giả lập, VNPay sandbox), bấm **Đặt hàng**.
5. **Đơn hàng của tôi:** xem lịch sử đơn; mở một đơn để theo dõi trạng thái cập nhật tự động; **Hủy đơn** khi đơn đang chờ xác nhận.
6. **Tài khoản:** xem và sửa hồ sơ, đổi mật khẩu.

### 14.2. Dành cho quản trị viên (Admin)

Đăng nhập bằng tài khoản Admin (email và mật khẩu khai báo trong `.env`); hệ thống tự chuyển tới `/admin`.

| Trang | Công việc |
|---|---|
| **Dashboard** | Xem tổng số đơn, doanh thu, đơn theo trạng thái, sản phẩm bán chạy, sản phẩm sắp hết hàng; chọn khoảng ngày để xem doanh thu |
| **Sản phẩm** | Thêm, sửa giá và tồn kho, ngừng bán/bán lại, đặt khuyến mãi theo sản phẩm (phần trăm và thời gian hiệu lực) |
| **Voucher** | Tạo voucher (loại, giá trị, giá trị đơn tối thiểu, giảm tối đa, số lượt, hạn dùng), bật/tắt, theo dõi lượt đã dùng |
| **Đơn hàng** | Lọc theo trạng thái, ngày, mã đơn; xem chi tiết; cập nhật trạng thái sang bước kế tiếp (chế độ `manual`); hủy đơn đang chờ xác nhận |
| **Người dùng** | Tìm kiếm tài khoản, khóa/mở khóa tài khoản Customer |

### 14.3. Dữ liệu mẫu để thử nghiệm

| Dữ liệu | Mô tả |
|---|---|
| Tài khoản Admin | Tạo từ `ADMIN_EMAIL`, `ADMIN_PASSWORD` trong `.env` |
| Tài khoản Customer mẫu | Tạo bởi `scripts/seed-demo-data.sh`; email và mật khẩu được in ra khi chạy script |
| Voucher `GIAM10` | Giảm 10%, tối đa theo cấu hình, đơn từ mức tối thiểu cấu hình |
| Voucher `GIAM50K` | Giảm 50.000đ, đơn từ mức tối thiểu cấu hình |
| Voucher `HETHAN` | Voucher đã hết hạn, dùng để kiểm tra thông báo lỗi |

### 14.4. Các trường hợp hệ thống xử lý

| Tình huống | Cách xử lý |
|---|---|
| Đăng ký email đã tồn tại | `EMAIL_EXISTS` |
| Mật khẩu yếu | `WEAK_PASSWORD`, nêu rõ yêu cầu |
| Sai email hoặc mật khẩu | `INVALID_CREDENTIALS`, thông báo chung không lộ email nào tồn tại |
| Đăng nhập sai nhiều lần liên tiếp | Khóa tạm, trả `ACCOUNT_LOCKED` |
| Tài khoản bị Admin khóa | Không đăng nhập được; token đang dùng bị từ chối ngay ở request kế tiếp |
| Token hết hạn hoặc bị sửa | 401 (`TOKEN_EXPIRED`/`UNAUTHORIZED`), giao diện chuyển về trang đăng nhập |
| Customer gọi API quản trị | 403 `FORBIDDEN` |
| Customer xem đơn của người khác | `NOT_FOUND` |
| Admin gọi chức năng giỏ hàng, đặt hàng | 403 `FORBIDDEN` |
| Customer gõ đường dẫn `/admin` | Trang 403 |
| Số lượng ≤ 0 hoặc không phải số | Từ chối, trả `VALIDATION_ERROR` |
| Số lượng vượt tồn kho | Từ chối, báo số lượng còn lại (`OUT_OF_STOCK`) |
| Thêm mặt hàng đã có | Cộng dồn số lượng, kiểm tra tồn kho sau khi cộng |
| Xóa/sửa/ghim mặt hàng không tồn tại | `NOT_FOUND`, không làm hỏng danh sách |
| Thao tác ở node đầu, node cuối, node duy nhất | Cập nhật đúng `dau`, `cuoi` |
| Dời lên khi đã ở đầu, dời xuống khi đã ở cuối | Bỏ qua, không lỗi |
| Undo khi chưa có thao tác / Redo khi không còn nhánh | `NOTHING_TO_UNDO` / `NOTHING_TO_REDO` |
| Tạo đơn khi không chọn mặt hàng nào | `EMPTY_SELECTION` |
| Voucher sai, hết hạn, hết lượt, chưa đủ giá trị tối thiểu | `VOUCHER_INVALID`, `VOUCHER_EXPIRED`, `MIN_ORDER_NOT_MET` |
| Giá hoặc tồn kho thay đổi giữa lúc xem giỏ và lúc đặt | Kiểm tra lại khi tạo đơn, báo người dùng nếu có thay đổi |
| Admin ngừng bán sản phẩm đang nằm trong giỏ | Giỏ hiển thị cảnh báo, không cho đặt mặt hàng đó |
| Admin chuyển trạng thái đơn sai thứ tự | `INVALID_STATE` |
| Thanh toán thất bại hoặc người dùng hủy | Đơn giữ nguyên, cho phép thanh toán lại hoặc đổi phương thức |
| Mất kết nối WebSocket | Tự kết nối lại và đồng bộ trạng thái hiện tại |
| Hủy đơn không ở trạng thái chờ xác nhận | `INVALID_STATE` |

---

## 15. Lộ trình 4 phiên bản

Mỗi phiên bản là một bản **chạy được hoàn chỉnh** và được gắn tag Git (`v1.0.0`, `v2.0.0`, …). Phiên bản sau kế thừa toàn bộ phiên bản trước.

| Phiên bản | Tên | Trọng tâm | Chức năng |
|---|---|---|---|
| **v1.0** | Core | Đáp ứng đề tài với danh sách liên kết đôi | F01–F05 |
| **v2.0** | Stable | Cơ sở dữ liệu, tài khoản và phân quyền, thao tác nâng cao trên giỏ, quản lý sản phẩm | F06, F07, F08, F13, F14, F15, F19, F20, F21, F24 |
| **v3.0** | Advanced | Khuyến mãi, Undo/Redo, đơn hàng, thời gian thực, quản trị voucher và đơn | F09, F10, F11, F12, F16 (COD, giả lập), F17, F23, F25, F26 |
| **v4.0** | Final | Thanh toán sandbox, thống kê, quản lý người dùng, bảo mật, hoàn thiện | F16 (VNPay sandbox), F18, F22, F27, F28 |

---

### 🔹 v1.0 – Core

**Mục tiêu:** đáp ứng đầy đủ yêu cầu bắt buộc của đề tài; có thể demo ngay.

**Phạm vi:**
- Backend: `CartList` (danh sách liên kết đôi), API thêm/xem/sửa/xóa, tạo đơn.
- Dữ liệu sản phẩm nạp từ file JSON; **một giỏ hàng demo duy nhất** và đơn hàng lưu trong bộ nhớ; chưa có đăng nhập.
- Frontend: trang Sản phẩm, trang Giỏ hàng, trang Xác nhận đơn.

**Cấu trúc thư mục:**

```
online-cart-system/
├── README.md
├── .gitignore
├── docker-compose.yml
├── backend/
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── config/config.json
│   ├── data/products.json                  # dữ liệu sản phẩm mẫu
│   ├── src/
│   │   ├── main.cc
│   │   ├── domain/ (CartNode.h, CartList.h/.cc)
│   │   ├── controllers/ (ProductController, CartController, OrderController, HealthController)
│   │   ├── services/ (CartService, OrderService)
│   │   ├── dto/ (CartDto.h, ApiResponse.h)
│   │   └── utils/ (Money.h, Validator.h/.cc)
│   └── tests/unit/ (CartListTest.cc)
├── frontend/
│   ├── package.json, tsconfig.json, vite.config.ts, tailwind.config.js
│   └── src/
│       ├── main.tsx, App.tsx, routes.tsx
│       ├── api/ (client.ts, cartApi.ts, orderApi.ts)
│       ├── types/ (cart.ts, order.ts, api.ts)
│       ├── components/ (layout, product, cart, common)
│       └── pages/ (ProductsPage, CartPage, OrderConfirmPage)
└── docs/ (requirements.md, data-structures.md)
```

**Tiêu chí hoàn thành (Definition of Done):**
- [ ] F01–F05 hoạt động qua giao diện và Postman.
- [ ] `CartListTest` bao phủ: giỏ rỗng, 1 phần tử, xóa đầu/cuối/giữa, thêm trùng `id`.
- [ ] Không còn rò rỉ bộ nhớ trong `CartList` (kiểm tra bằng công cụ phân tích bộ nhớ).
- [ ] Chạy được bằng `docker compose up`.
- [ ] Có tài liệu mô tả cấu trúc dữ liệu và độ phức tạp.

---

### 🔹 v2.0 – Stable

**Mục tiêu:** hệ thống ổn định, dữ liệu không mất, có tài khoản và phân quyền Customer/Admin, kiểm soát được đầu vào và trường hợp biên.

**Phạm vi:**
- Tích hợp **PostgreSQL**, write-through từ `CartList` xuống `cart_items`, dựng lại danh sách khi khởi động.
- **Đăng ký, đăng nhập (JWT), phân quyền Customer/Admin**; giỏ hàng riêng theo tài khoản; tạo Admin ban đầu từ biến môi trường.
- Giao diện hai layout: `CustomerLayout` và `AdminLayout`; bảo vệ route theo vai trò; trang 403/404.
- Admin: **quản lý sản phẩm** (thêm, sửa, tồn kho, ngừng bán).
- Giỏ hàng: ghim, dời lên/xuống, chọn/bỏ chọn, kiểm tra tồn kho, sắp xếp, tìm kiếm, lọc.
- Kiểm tra đầu vào tập trung, định dạng lỗi thống nhất, transaction khi tạo đơn.
- Bộ test đơn vị và tích hợp (gồm ma trận quyền); collection Postman theo vai trò.

**Thay đổi so với v1.0** (`+` thêm mới, `~` cập nhật, `−` bỏ):

```
~ docker-compose.yml                         # thêm dịch vụ PostgreSQL
+ .env.example                               # DB, JWT, Admin ban đầu, CORS
− backend/data/products.json                 # thay bằng cơ sở dữ liệu
+ database/
│   ├── schema.sql
│   ├── migrations/ (001_create_users.sql, 002_create_products.sql,
│   │                003_create_carts.sql, 004_create_orders.sql)
│   └── seed/products.sql
+ backend/src/domain/ (CartManager.h/.cc, Role.h)      # userId → CartList
~ backend/src/domain/CartList.cc             # + ghimLenDau, doiLen, doiXuong, sapXep, chonMatHang
+ backend/src/controllers/ (AuthController, AccountController)   # AccountController: xem hồ sơ
+ backend/src/controllers/admin/AdminProductController.h/.cc
+ backend/src/services/ (AuthService, JwtService, PasswordHasher, UserService)
+ backend/src/repositories/ (UserRepo, ProductRepo, CartRepo, OrderRepo)
+ backend/src/filters/ (CorsFilter, AuthFilter, RoleFilter)
+ backend/src/dto/ (AuthDto.h, AdminDto.h)
~ backend/src/services/CartService.cc        # theo userId; + tồn kho, ghim, dời, chọn, sắp xếp, lọc
~ backend/src/services/OrderService.cc       # gắn user_id, transaction tạo đơn, trừ tồn kho
~ backend/src/main.cc                        # tạo Admin ban đầu nếu chưa có
~ backend/src/utils/Validator.cc             # + email, độ mạnh mật khẩu
+ backend/tests/unit/ (ValidatorTest, PasswordHasherTest, JwtServiceTest)
~ backend/tests/unit/CartListTest.cc         # + ghim, dời, sắp xếp
+ backend/tests/integration/ (AuthApiTest, RoleAccessTest, OwnershipTest, CartApiTest, OrderFlowTest, AdminApiTest)
+ api/postman/ (online-cart.postman_collection.json, local.postman_environment.json)
+ frontend/src/context/ (AuthContext.tsx, CartContext.tsx)
+ frontend/src/hooks/ (useAuth.ts, useCart.ts)
+ frontend/src/api/ (authApi.ts, accountApi.ts, adminApi.ts)
~ frontend/src/api/client.ts                 # gắn Authorization, xử lý 401/403
+ frontend/src/types/ (auth.ts, admin.ts)
+ frontend/src/utils/tokenStorage.ts
+ frontend/src/components/layout/ (CustomerLayout, AdminLayout, Header, AdminSidebar, Footer)
+ frontend/src/components/auth/ (ProtectedRoute, AdminRoute, GuestRoute, LoginForm, RegisterForm, PasswordInput)
+ frontend/src/components/admin/ (DataTable, ProductForm, ConfirmDialog)
+ frontend/src/components/cart/ (CartToolbar, CartItemRow)     # ghim, dời, tick chọn, tìm, lọc, sắp xếp
+ frontend/src/components/common/ (ErrorBanner, Spinner, EmptyState, Modal)
+ frontend/src/pages/ (LoginPage, RegisterPage, ForbiddenPage, NotFoundPage)
+ frontend/src/pages/admin/AdminProductsPage.tsx
~ frontend/src/routes.tsx                    # route theo vai trò
+ docs/ (architecture.md, auth-and-roles.md, database.md, api.md, test-plan.md)
+ scripts/ (setup.sh, test-all.sh)
+ .github/workflows/ (backend-ci.yml, frontend-ci.yml)
```

**Tiêu chí hoàn thành:**
- [ ] Đăng ký, đăng nhập, đăng xuất hoạt động; mật khẩu lưu dạng băm Argon2id; không có mật khẩu Admin mặc định trong mã nguồn.
- [ ] `RoleAccessTest` kiểm tra **mọi endpoint × (khách, Customer, Admin)** đúng mã 200/401/403 theo [ma trận phân quyền](#72-ma-trận-phân-quyền).
- [ ] `OwnershipTest`: Customer A không đọc, sửa được dữ liệu của Customer B.
- [ ] Giao diện Customer và Admin tách biệt; Customer vào `/admin` thấy trang 403; chưa đăng nhập bị chuyển về `/login`.
- [ ] Khởi động lại server, giỏ hàng của từng tài khoản giữ nguyên thứ tự và nội dung.
- [ ] F06, F07, F08, F13, F14, F15, F24 hoạt động; mọi API đầu vào sai đều trả lỗi chuẩn, không làm sập server.
- [ ] Bảng test case có đủ: dữ liệu hợp lệ, trường hợp biên, dữ liệu không hợp lệ.
- [ ] Giỏ hàng truy cập đồng thời không gây lỗi dữ liệu (kiểm tra bằng test đồng thời).
- [ ] CI biên dịch và chạy test tự động trên mỗi Pull Request.

---

### 🔹 v3.0 – Advanced

**Mục tiêu:** bổ sung các chức năng tạo trải nghiệm mua sắm thực tế và công cụ vận hành cho Admin.

**Phạm vi:**
- Voucher, khuyến mãi theo mặt hàng, `PricingService` với công thức tính tiền chuẩn, gợi ý tiết kiệm.
- **Undo/Redo** bằng `HistoryList`.
- Thanh toán **COD** và **cổng giả lập (mock)**.
- Trạng thái đơn chuyển tự động hoặc thủ công (`ORDER_FLOW_MODE`), **theo dõi thời gian thực qua WebSocket** có xác thực.
- Customer: **lịch sử đơn hàng của tôi**.
- Admin: **quản lý voucher**, **quản lý đơn hàng** (xem tất cả, cập nhật trạng thái).

**Thay đổi so với v2.0:**

```
+ database/migrations/ (005_create_vouchers.sql, 006_create_payments.sql,
│                       007_add_product_promotion.sql, 008_create_order_status_history.sql)
+ database/seed/vouchers.sql
+ backend/src/domain/ (HistoryNode.h, HistoryList.h/.cc)
~ backend/src/domain/CartManager.cc          # gắn HistoryList cho mỗi giỏ
+ backend/src/services/ (PricingService, VoucherService, PaymentService, IPaymentGateway,
│                        MockGateway, TrackingService)
+ backend/src/repositories/ (VoucherRepo, PaymentRepo)
+ backend/src/controllers/ (PaymentController, OrderWebSocketController)
+ backend/src/controllers/admin/ (AdminVoucherController, AdminOrderController)
~ backend/src/controllers/CartController.cc  # + undo, redo, voucher, summary
~ backend/src/controllers/OrderController.cc # + danh sách đơn của tôi
~ backend/src/controllers/admin/AdminProductController.cc   # + khuyến mãi theo sản phẩm
~ backend/src/services/CartService.cc        # đẩy thao tác vào HistoryList
~ backend/src/services/OrderService.cc       # snapshot giá, voucher, phí vận chuyển; máy trạng thái
+ backend/tests/unit/ (HistoryListTest, PricingServiceTest, VoucherServiceTest, OrderStateMachineTest)
+ backend/tests/integration/ (PaymentFlowTest, TrackingTest)
~ backend/tests/integration/ (RoleAccessTest, OwnershipTest, AdminApiTest)   # bổ sung endpoint mới, kiểm tra quyền WebSocket
+ frontend/src/components/cart/ (CartSummary, SavingHint, UndoRedoBar, VoucherInput)
+ frontend/src/components/checkout/ (PaymentMethodPicker, OrderReview)
+ frontend/src/components/order/ (OrderTimeline, OrderStatusBadge)
+ frontend/src/components/admin/ (VoucherForm, OrderStatusControl)
+ frontend/src/api/ (paymentApi.ts, orderSocket.ts)
+ frontend/src/hooks/ (useOrderTracking.ts, useToast.ts)
+ frontend/src/pages/ (CheckoutPage, MyOrdersPage, OrderTrackingPage)
+ frontend/src/pages/admin/ (AdminVouchersPage, AdminOrdersPage)
~ api/postman/online-cart.postman_collection.json
+ docs/ (ui-guidelines.md)
~ docs/ (api.md, test-plan.md, data-structures.md, auth-and-roles.md)
```

**Tiêu chí hoàn thành:**
- [ ] F09, F10, F11, F12, F16 (COD, giả lập), F17, F23, F25, F26 hoạt động end-to-end.
- [ ] Undo/Redo đúng cho mọi loại thao tác; thao tác mới sau Undo xóa nhánh Redo.
- [ ] Tổng tiền khớp công thức ở [mục 8.2](#82-giải-thuật-chính) trong mọi tổ hợp voucher, khuyến mãi, mặt hàng được chọn.
- [ ] Customer chỉ nhận cập nhật WebSocket của đơn mình; kết nối không token hoặc sai chủ đơn bị từ chối.
- [ ] Ở chế độ `manual`, Admin cập nhật trạng thái và Customer thấy thay đổi ngay không cần tải lại trang; chuyển sai thứ tự bị từ chối.
- [ ] Mất kết nối WebSocket tự phục hồi.
- [ ] Tạo đơn, trừ tồn kho, tăng lượt dùng voucher nằm trong một transaction (kiểm tra bằng test lỗi giữa chừng).

---

### 🔹 v4.0 – Final

**Mục tiêu:** sản phẩm hoàn chỉnh, ổn định, giao diện hoàn thiện, bảo mật cơ bản đầy đủ, sẵn sàng demo và bảo vệ.

**Phạm vi:**
- Thanh toán **VNPay môi trường sandbox**: tạo URL có chữ ký, xử lý return URL và IPN, kiểm tra chữ ký và số tiền, xử lý idempotent.
- Hủy đơn hàng (F18, cả Customer và Admin).
- Admin: **Dashboard thống kê** (F27), **quản lý người dùng** khóa/mở khóa (F28).
- Tài khoản: **hồ sơ và đổi mật khẩu** (F22); **khóa tạm khi đăng nhập sai nhiều lần**.
- Hoàn thiện giao diện: responsive trên điện thoại/máy tính bảng/máy tính, trạng thái tải, trạng thái rỗng, thông báo lỗi thân thiện, hỗ trợ bàn phím và nhãn truy cập.
- Đo hiệu năng cấu trúc dữ liệu, rà soát bảo mật cơ bản, hoàn thiện tài liệu và kịch bản demo.

**Thay đổi so với v3.0:**

```
+ LICENSE, .editorconfig
+ database/migrations/009_add_login_lockout.sql          # failed_attempts, locked_until
+ backend/src/services/ (VnpayGateway.h/.cc, StatsService.h/.cc)
+ backend/src/repositories/StatsRepo.h/.cc
+ backend/src/controllers/admin/ (AdminUserController, AdminStatsController)
~ backend/src/services/AuthService.cc        # khóa tạm khi sai mật khẩu nhiều lần
~ backend/src/services/UserService.cc        # đổi mật khẩu, khóa/mở khóa tài khoản
~ backend/src/controllers/AccountController.cc           # + sửa hồ sơ, đổi mật khẩu
~ backend/src/services/PaymentService.cc     # chọn cổng theo phương thức, idempotent
~ backend/src/controllers/PaymentController.cc           # + /vnpay/return, /vnpay/ipn
~ backend/src/controllers/OrderController.cc             # + hủy đơn
~ backend/src/controllers/admin/AdminOrderController.cc  # + hủy đơn
~ backend/src/services/OrderService.cc       # + hủy đơn, hoàn lại tồn kho và lượt voucher
+ backend/src/utils/Logger.h                 # log có mức độ, không ghi mật khẩu/token
+ backend/tests/integration/ (VnpayCallbackTest, CancelOrderTest)
~ backend/tests/integration/ (AuthApiTest, RoleAccessTest, AdminApiTest)   # khóa tạm, khóa người dùng, thống kê
+ backend/tests/performance/CartListBenchmark.cc
+ frontend/src/pages/ (PaymentResultPage, AccountPage)
+ frontend/src/pages/admin/ (AdminDashboardPage, AdminUsersPage)
+ frontend/src/components/admin/ (StatCard, BarChart)    # biểu đồ SVG tự vẽ, không thêm thư viện
~ frontend/src/components/**                 # hoàn thiện responsive, trạng thái tải/rỗng/lỗi
+ frontend/src/__tests__/                    # route guard, form đăng nhập, thành phần chính
+ scripts/ (dev.sh, seed-demo-data.sh)
+ .github/ (ISSUE_TEMPLATE/, pull_request_template.md)
+ docs/ (performance-report.md)
+ docs/report/bao-cao-do-an.docx
+ docs/demo/kich-ban-demo.md
+ docs/team/phan-cong.md
~ README.md                                   # bản hoàn chỉnh
```

**Tiêu chí hoàn thành:**
- [ ] Toàn bộ F01–F28 hoạt động; luồng thanh toán VNPay sandbox thành công, thất bại và hủy đều xử lý đúng.
- [ ] Admin khóa tài khoản Customer: tài khoản đó không đăng nhập được và token đang dùng bị từ chối ngay.
- [ ] Số liệu Dashboard khớp với dữ liệu đơn hàng trong cơ sở dữ liệu (kiểm tra bằng test đối chiếu).
- [ ] Đăng nhập sai quá số lần cấu hình thì bị khóa tạm đúng thời gian.
- [ ] Giao diện hiển thị đúng ở các kích thước màn hình phổ biến, cả giao diện Customer và Admin.
- [ ] Toàn bộ test (đơn vị, tích hợp, frontend) đạt; CI xanh trên nhánh `main`.
- [ ] Có số liệu hiệu năng cho n = 10, 100, 1000, 10000 mặt hàng trong `performance-report.md`.
- [ ] Chạy sạch từ đầu chỉ bằng `docker compose up --build` trên máy mới.
- [ ] Có kịch bản demo cho cả hai vai trò, danh sách câu hỏi vấn đáp và báo cáo hoàn chỉnh.

---

## 16. Kiểm thử và đảm bảo chất lượng

### 16.1. Các tầng kiểm thử

| Tầng | Công cụ | Nội dung |
|---|---|---|
| Đơn vị (backend) | GoogleTest | `CartList`, `HistoryList`, `PricingService`, `VoucherService`, `Validator`, `PasswordHasher`, `JwtService`, máy trạng thái đơn |
| Tích hợp (backend) | GoogleTest + PostgreSQL thử nghiệm | Xác thực, ma trận quyền, quyền sở hữu dữ liệu, giỏ hàng, đơn hàng, thanh toán, theo dõi đơn, API Admin |
| API | Postman | Bộ request chia theo thư mục Auth/Customer/Admin, gồm cả trường hợp lỗi, dùng `tokenCustomer` và `tokenAdmin` |
| Frontend | Vitest + React Testing Library | Route guard theo vai trò, form đăng nhập/đăng ký, thành phần giỏ hàng, hiển thị lỗi, theo dõi đơn |
| Hiệu năng | `CartListBenchmark` | Thời gian các thao tác với n = 10, 100, 1000, 10000 |
| Thủ công | Kịch bản demo | Duyệt toàn bộ luồng của Customer và Admin trên giao diện |

### 16.2. Nhóm test case bắt buộc

**Cấu trúc dữ liệu**

| Nhóm | Ví dụ |
|---|---|
| Giỏ rỗng | Xem, xóa, ghim, dời, Undo trên giỏ rỗng |
| Một phần tử | Xóa, ghim, dời, sắp xếp khi chỉ có 1 node |
| Vị trí biên | Xóa/ghim/dời node đầu, node cuối, node giữa |
| Dữ liệu không hợp lệ | Số lượng âm, bằng 0, quá lớn, `id` không tồn tại |
| Undo/Redo | Undo nhiều bước rồi thao tác mới; Redo khi hết nhánh |
| Tính tiền | Không voucher, voucher % có trần, voucher cố định lớn hơn tạm tính, khuyến mãi hết hạn |
| Đồng thời | Nhiều request cùng sửa một giỏ |

**Xác thực và phân quyền**

| Nhóm | Ví dụ |
|---|---|
| Đăng ký | Email trùng, email sai định dạng, mật khẩu yếu, cố gửi `role=ADMIN` khi đăng ký (phải bị bỏ qua) |
| Đăng nhập | Đúng/sai mật khẩu, email không tồn tại (cùng thông báo), tài khoản bị khóa, khóa tạm sau nhiều lần sai |
| Token | Hợp lệ, hết hạn, sai chữ ký, bị sửa `role` trong payload, thiếu header |
| Ma trận quyền | Mỗi endpoint × (khách, Customer, Admin): đúng 200/401/403 |
| Quyền sở hữu | Customer A đọc, hủy, thanh toán đơn của Customer B → `NOT_FOUND`; Customer A mở WebSocket đơn của B → từ chối |
| Giao diện | Customer vào `/admin` → 403; khách vào `/cart` → `/login`; đăng nhập xong chuyển đúng theo vai trò; phiên hết hạn chuyển về `/login` |
| Quản trị | Admin không khóa được chính mình hoặc Admin khác; Admin đặt hàng/dùng giỏ → 403 |

### 16.3. Tiêu chí chất lượng

- Mọi Pull Request phải qua CI (biên dịch, lint, test) trước khi hợp nhất.
- Không hợp nhất khi còn test thất bại.
- Mọi endpoint mới phải được thêm vào `RoleAccessTest` trước khi hợp nhất.
- Giao diện luôn hiển thị trạng thái tải, trạng thái rỗng và thông báo lỗi dễ hiểu; không để lộ thông báo kỹ thuật cho người dùng.

---

## 17. Quy trình phát triển

### 17.1. Chiến lược nhánh Git

| Nhánh | Mục đích |
|---|---|
| `main` | Bản ổn định, mỗi mốc phiên bản gắn tag |
| `develop` | Nhánh tích hợp |
| `feature/<mã>-<mô-tả>` | Phát triển một chức năng, ví dụ `feature/F10-undo-redo`, `feature/F21-role-based-access` |
| `fix/<mô-tả>` | Sửa lỗi |
| `release/vX.Y` | Chuẩn bị phát hành phiên bản |

### 17.2. Quy ước

- **Commit** theo Conventional Commits: `feat:`, `fix:`, `test:`, `docs:`, `refactor:`, `chore:`.
- **Pull Request** cần ít nhất một thành viên khác xem xét trước khi hợp nhất vào `develop`; Pull Request chạm vào xác thực/phân quyền cần xem xét kỹ phần chính sách route.
- **Bảo mật mã nguồn:** không commit `.env`, khóa, mật khẩu; kiểm tra trước khi push.
- **Định dạng mã:** `clang-format` cho C++, ESLint + Prettier cho TypeScript.
- **Đặt tên:** thành phần React theo PascalCase, hàm/biến TypeScript theo camelCase; lớp C++ theo PascalCase, trường dữ liệu của cấu trúc dữ liệu cốt lõi dùng tên tiếng Việt không dấu theo camelCase (`tenHang`, `soLuong`).

### 17.3. Quản lý công việc

GitHub Projects với bảng cột: `Backlog` → `Đang làm` → `Đang review` → `Đã kiểm thử` → `Hoàn thành`. Mỗi chức năng (F01–F28) là một issue, gắn nhãn phiên bản (`v1.0`–`v4.0`), nhãn khu vực (`customer`, `admin`, `auth`, `core-ds`) và người phụ trách.

---

## 18. Quản lý rủi ro

| Rủi ro | Mức độ | Biện pháp |
|---|---|---|
| Môi trường biên dịch Drogon khác nhau giữa các máy | Cao | Dùng Docker Compose làm môi trường chuẩn; Dockerfile cố định phiên bản phụ thuộc |
| Lỗi con trỏ (rò rỉ bộ nhớ, truy cập null) trong danh sách liên kết | Cao | Test biên bắt buộc; kiểm tra bằng công cụ phân tích bộ nhớ; viết và kiểm thử `CartList` trước khi làm API |
| **Lỗ hổng phân quyền** (Customer gọi API Admin, đọc dữ liệu người khác) | Cao | Phân quyền ở backend bằng `AuthFilter` + `RoleFilter`; `RoleAccessTest` và `OwnershipTest` bắt buộc; chặn mặc định (route mới không khai báo chính sách thì từ chối) |
| **Lộ khóa hoặc mật khẩu** (`JWT_SECRET`, mật khẩu Admin, khóa VNPay) | Cao | Chỉ đọc từ biến môi trường; `.env` nằm trong `.gitignore`; `setup.sh` sinh khóa ngẫu nhiên; không ghi log dữ liệu nhạy cảm |
| Mật khẩu bị lưu hoặc xử lý sai cách | Cao | Argon2id qua libsodium, không tự cài đặt thuật toán băm; có `PasswordHasherTest` |
| Truy cập đồng thời gây hỏng danh sách | Trung bình | Khóa `std::mutex` theo từng giỏ; test đồng thời |
| Tích hợp cổng thanh toán tốn thời gian | Trung bình | Đặt ở v4.0; từ v3.0 đã có COD và cổng giả lập hoàn chỉnh; VNPay đi sau giao diện `IPaymentGateway` nên thêm vào không ảnh hưởng phần khác |
| WebSocket mất kết nối hoặc bị truy cập trái phép | Trung bình | Client tự kết nối lại và tải lại trạng thái qua REST; xác thực token và kiểm tra chủ đơn khi kết nối |
| Lệch dữ liệu giữa bộ nhớ và PostgreSQL | Trung bình | Write-through trong transaction; khi lỗi ghi thì hoàn tác thao tác trong bộ nhớ |
| v2.0 có khối lượng lớn nhất (cơ sở dữ liệu, xác thực, phân quyền, quản lý sản phẩm, thao tác nâng cao) | Cao | Chia việc theo mảng độc lập (xác thực/phân quyền, giao diện Admin, cấu trúc dữ liệu giỏ hàng); làm xác thực và `CartList` trước, thao tác nâng cao sau |
| Thành viên không nắm được phần mình làm | Cao | Mỗi chức năng có người phụ trách, tài liệu giải thích và test case đi kèm; rà soát chéo qua Pull Request |
| Không kịp tiến độ | Trung bình | Mỗi phiên bản là bản chạy được hoàn chỉnh; **v1.0 đã đáp ứng yêu cầu bắt buộc của đề tài**; nếu thiếu thời gian, cắt theo thứ tự ưu tiên ngược: F28, F27, F22 → VNPay sandbox → F25, F26 → các chức năng còn lại |

---

## 19. Khai thác và sử dụng AI

Nhóm sử dụng công cụ AI để hỗ trợ: gợi ý phân tích yêu cầu, đề xuất cấu trúc dữ liệu và thuật toán, rà soát lỗi, sinh test case và hỗ trợ viết tài liệu.

Nguyên tắc: **mọi nội dung do AI hỗ trợ đều được thành viên phụ trách đọc, kiểm tra, chạy thử và hiểu rõ trước khi đưa vào sản phẩm**; riêng phần xác thực, phân quyền và băm mật khẩu được kiểm tra thêm bằng bộ test ở mục 16.2 và rà soát chéo giữa các thành viên. Mỗi thành viên chịu trách nhiệm giải thích phần việc của mình trong buổi vấn đáp. Chi tiết công cụ, mục đích và cách kiểm chứng được ghi trong báo cáo (`docs/report/`).

---

## 20. Hướng phát triển

| Hướng | Nội dung |
|---|---|
| Nâng cấp xác thực | Refresh token trong cookie `HttpOnly`, thu hồi token, quên mật khẩu qua email, xác minh email, xác thực hai lớp, đăng nhập bằng Google |
| Phân quyền chi tiết | Thêm vai trò nhân viên (staff) với quyền theo từng module, nhật ký thao tác của Admin |
| Thanh toán thực tế | Chuyển từ sandbox sang môi trường chính thức; bổ sung MoMo, ZaloPay; hoàn tiền |
| Vận chuyển | Tích hợp đơn vị vận chuyển để tính phí theo địa chỉ và theo dõi hành trình thật; quản lý nhiều địa chỉ giao hàng |
| Quản trị mở rộng | Báo cáo xuất file, quản lý danh mục, nhập/xuất tồn kho |
| Gợi ý sản phẩm | Gợi ý mua kèm dựa trên nội dung giỏ hàng |
| Hiệu năng | Thay bubble sort bằng merge sort cho danh sách liên kết; bộ nhớ đệm (cache) cho danh mục sản phẩm |
| Mở rộng hệ thống | Nhiều phiên bản backend dùng chung bộ nhớ đệm giỏ hàng; hàng đợi xử lý đơn |
| Trải nghiệm | Ứng dụng di động, thông báo đẩy khi đơn đổi trạng thái, đa ngôn ngữ |
| Bảo mật | Giới hạn tần suất truy cập (rate limit) theo IP, kiểm toán log, quét lỗ hổng phụ thuộc |

---

<p align="center"><i>Học phần: Cấu trúc dữ liệu và giải thuật – Đồ án cuối kì</i></p>
