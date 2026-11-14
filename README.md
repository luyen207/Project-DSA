# ShopPro – Hệ thống quản lý giỏ hàng mua sắm trực tuyến

**ShopPro** là một cửa hàng trực tuyến hoàn chỉnh: khách hàng xem sản phẩm, bỏ vào giỏ, nhập mã giảm giá và đặt hàng;
quản trị viên theo dõi đơn, kho, khách hàng, báo cáo và nhật ký hoạt động. Toàn bộ hệ thống chạy bằng **một máy chủ C++ duy nhất**.

Đây là đồ án nhóm môn **Cấu trúc dữ liệu & Giải thuật**. Điểm cốt lõi: **giỏ hàng của mỗi khách là một danh sách liên kết đôi do nhóm tự cài đặt**
(thêm, xóa, sắp xếp, tìm kiếm, duyệt xuôi/ngược, tự kiểm tra tính toàn vẹn của liên kết). Mọi thứ còn lại (đăng nhập, đơn hàng, mã giảm giá, báo cáo, giao diện…) được xây xung quanh lõi đó.

| Thành phần | Mô tả |
|---|---|
| **Máy chủ** | C++17, REST API + phục vụ luôn các trang web, lưu dữ liệu bằng tệp JSON (không dùng cơ sở dữ liệu SQL) |
| **Giao diện khách hàng** | HTML/CSS/JavaScript thuần, nhiều trang, dùng được trên điện thoại |
| **Giao diện quản trị** | Bảng điều khiển có biểu đồ, quản lý sản phẩm, đơn hàng, khách hàng, mã giảm giá, báo cáo, nhật ký |

Quy mô hiện tại: khoảng 1.300 dòng C++ cho lõi DSA, 4.000 dòng C++ cho máy chủ và API, 2.000 dòng kiểm thử, 3.300 dòng JavaScript/CSS cho giao diện.

## Mục lục

1. [Chạy nhanh](#1-chạy-nhanh) · 2. [Chức năng](#2-chức-năng) · 3. [Giỏ hàng và cấu trúc dữ liệu](#3-giỏ-hàng-và-cấu-trúc-dữ-liệu) · 4. [Kiến trúc](#4-kiến-trúc) ·
5. [Mã giảm giá](#5-mã-giảm-giá) · 6. [Dữ liệu](#6-dữ-liệu-và-lưu-trữ) · 7. [REST API](#7-rest-api) · 8. [Bảo mật](#8-bảo-mật) ·
9. [Kiểm thử](#9-đã-kiểm-tra-những-gì-và-chưa-kiểm-tra-gì) · 10. [Kịch bản demo](#10-kịch-bản-demo-hai-màn-hình-5-phút) · 11. [Cấu trúc thư mục](#11-cấu-trúc-thư-mục) ·
12. [Tài liệu đi kèm](#12-tài-liệu-đi-kèm) · 13. [Giới hạn đã biết](#13-giới-hạn-đã-biết)

---

## 1. Chạy nhanh

**Yêu cầu:** trình biên dịch C++17 (GCC ≥ 9 hoặc Clang ≥ 10), **CMake ≥ 3.18**, **OpenSSL** (gói phát triển, ví dụ `libssl-dev`). Thư viện JSON đã kèm sẵn trong `Backend/third_party`, nên không cần Internet khi biên dịch.

```bash
bash scripts/build.sh --test      # biên dịch và chạy 7 chương trình kiểm thử (bỏ --test nếu chỉ cần biên dịch)
bash scripts/run.sh               # chạy máy chủ, mở http://127.0.0.1:8080/
```

Máy Windows: dùng **WSL (Ubuntu)**. Hướng dẫn từng bước từ máy chưa cài gì, dành cho các thành viên tự chạy:
**[docs/HUONG_DAN_CHAY_MAY_CHU.md](docs/HUONG_DAN_CHAY_MAY_CHU.md)**.
Hướng dẫn biên dịch chi tiết và các tham số dòng lệnh: [Backend/README.md](Backend/README.md).

| Địa chỉ | Dành cho |
|---|---|
| `/` | Trang chọn giao diện |
| `/shop/` | Cửa hàng (khách hàng) |
| `/admin/` | Quản trị viên |
| `/api/...` | REST API (xem [docs/API.md](docs/API.md)) |

Dừng máy chủ bằng **Ctrl+C**. Lần chạy đầu, dữ liệu mẫu được sao sang `Backend/data/runtime/`; muốn quay về trạng thái ban đầu: `bash scripts/reset-data.sh`.

### Tài khoản demo (chỉ dùng để trình diễn)

| Vai trò | Email | Mật khẩu |
|---|---|---|
| Quản trị viên | `admin@shoppro.vn` | `Admin@12345` |
| Khách hàng | `khach@shoppro.vn` | `Demo@12345` |
| Khách hàng khác | `minhanh@example.com`, `quocbao@example.com`, `thuha@example.com`… | `Demo@12345` |
| Khách bị khóa (để demo) | `vankhoa@example.com` | `Demo@12345` |

Trang đăng nhập có nút **Điền sẵn**. Khi đưa lên mạng thật, hãy xóa khung "Tài khoản demo" trong `Frontend/shop/js/views/account.js` và `Frontend/admin/js/app.js`
rồi đổi mật khẩu Admin (xem [docs/TRIEN_KHAI_WEB.md](docs/TRIEN_KHAI_WEB.md)).

---

## 2. Chức năng

### Khách hàng (`/shop/`)

* Đăng ký, đăng nhập, cập nhật hồ sơ và địa chỉ giao hàng.
* Xem sản phẩm theo danh mục; **tìm kiếm không phân biệt dấu** (gõ `chuot` vẫn ra "Chuột"); lọc theo giá, còn hàng; sắp xếp; phân trang; trang chi tiết kèm sản phẩm liên quan.
* **Giỏ hàng:** thêm, đổi số lượng, xóa; sắp xếp theo giá / tên / mã; tìm trong giỏ; **xem cấu trúc danh sách liên kết** của chính giỏ mình (xem mục 3).
* **Mã giảm giá:** nhập mã hoặc bấm chọn mã gợi ý ở giỏ hàng và trang thanh toán; thấy ngay số tiền được giảm.
* Thanh toán (tiền mặt khi nhận hàng hoặc chuyển khoản), theo dõi tiến trình đơn, hủy đơn khi còn *Chờ xác nhận*.
* **Trợ lý mua sắm:** hiểu câu như "dưới 2 triệu", "500k", "rẻ nhất", "so sánh…". Chạy bằng bộ luật trên dữ liệu cửa hàng, không gọi dịch vụ bên ngoài.

### Quản trị viên (`/admin/`)

* **Tổng quan:** doanh thu, biểu đồ 14 ngày, trạng thái đơn, sản phẩm bán chạy, hàng sắp hết (mọi biểu đồ có chế độ xem dạng bảng).
* **Sản phẩm:** thêm, sửa, nhập kho, ẩn/hiện, xóa. Khách thấy thay đổi ngay. **Danh mục:** thêm, sửa, xóa.
* **Đơn hàng:** chuyển trạng thái theo đúng quy trình *Chờ xác nhận → Đã xác nhận → Đang giao → Hoàn thành*; hủy thì hàng được trả về kho; đơn hiện đủ tạm tính, mã giảm giá và tiền đã giảm. Chuyển sai quy trình bị từ chối.
* **Mã giảm giá:** tạo, sửa, bật/tắt, xóa; xem lượt dùng và tổng tiền đã giảm của từng mã.
* **Khách hàng:** xem số đơn và tổng chi tiêu; khóa/mở khóa (khóa là khách bị đăng xuất ngay).
* **Báo cáo:** theo khoảng ngày, xuất CSV. **Nhật ký hoạt động:** mọi thao tác của khách và Admin, có lọc. **Cài đặt cửa hàng.**
* **Thông báo đơn hàng mới:** khách vừa đặt hàng thì Admin thấy thông báo trong vòng khoảng 15 giây.

### Giao diện

Chữ lớn, rõ (không có chữ nhỏ hơn 14px), **không dùng màu tím**, dùng được trên màn hình điện thoại 390px không bị tràn ngang.
Font Inter và Lora (giấy phép OFL, kèm sẵn nên chạy được khi không có Internet); ảnh sản phẩm là SVG vẽ sẵn.

---

## 3. Giỏ hàng và cấu trúc dữ liệu

Mỗi khách có **một giỏ hàng riêng, là một danh sách liên kết đôi** (mỗi sản phẩm là một node có con trỏ `prev` và `next`, có `head` và `tail`).
Giỏ hàng được nạp vào danh sách khi cần và **ghi xuống tệp sau mỗi thao tác**, nên khởi động lại máy chủ không mất giỏ.

| Khách làm gì | API | Hàm trong lõi DSA |
|---|---|---|
| Thêm vào giỏ (trùng thì **cộng dồn** số lượng) | `POST /api/cart/items` | `ProductManager::themSanPham` |
| Đổi số lượng | `PATCH /api/cart/items/:id` | `capNhatSoLuong` |
| Bỏ khỏi giỏ | `DELETE /api/cart/items/:id` | `xoaSanPhamTheoID` |
| Sắp xếp theo giá / tên / mã | `POST /api/cart/sort` | `sapXepTheoGiaTangDan`, `sapXepTheoGiaGiamDan`, `sapXepTheoTen`, `sapXepTheoID` |
| Tìm trong giỏ (không dấu cũng được) | `GET /api/cart/search?q=` | `ProductSearch::timTheoTen` |
| Xem cấu trúc node, duyệt xuôi hoặc ngược | `GET /api/cart/structure?direction=` | duyệt từ head hoặc tail, kèm kết quả `kiemTraLienKet()` |

Trang *Giỏ hàng* có khung **"Giỏ hàng nhìn từ bên trong"**: mỗi sản phẩm hiện thành một node với `prev`/`next`, nhãn HEAD/TAIL, nút **Duyệt ngược** (đi từ tail về head),
và huy hiệu *Liên kết hợp lệ* chính là kết quả của hàm `kiemTraLienKet()`. Khi khách sắp xếp, các node đổi chỗ ngay trên màn hình.

Cách thức hoạt động khi ghi lỗi: nếu việc ghi tệp thất bại, danh sách trong RAM được **dựng lại từ dữ liệu cũ** (`CartEngine::rebuild`) để bộ nhớ và tệp không lệch nhau.
Trước khi trả giỏ, `syncWithCatalog` loại sản phẩm đã bị xóa hoặc ẩn và cập nhật tên, giá mới nhất từ kho.

Các lỗi đã sửa trong lõi DSA trước khi tích hợp (node lạ, giá trị NaN, tràn số, sao chép danh sách gây giải phóng hai lần…) nằm ở
[docs/NHAT_KY_DIEU_CHINH_LOI_DSA.md](docs/NHAT_KY_DIEU_CHINH_LOI_DSA.md); báo cáo audit ban đầu ở [docs/BAO_CAO_AUDIT_BACKEND.md](docs/BAO_CAO_AUDIT_BACKEND.md).

---

## 4. Kiến trúc

```
Trình duyệt ──HTTP──►  shoppro_api (C++17, một tiến trình)
 /shop/  (khách)        ├─ HttpServer      máy chủ HTTP/1.1 + phục vụ tệp tĩnh Frontend/
 /admin/ (quản trị)     ├─ Router          định tuyến, phân quyền Public / Customer / Admin
                        ├─ routes/*        nghiệp vụ: auth, giỏ hàng, đơn hàng, mã giảm giá, admin
                        ├─ Coupons         luật mã giảm giá (một nơi duy nhất)
                        ├─ CartEngine      giỏ hàng của từng khách = một CartController
                        │     └─ CartController ─► ProductManager · ProductSearch · CartProcessing
                        │                               └──────────► Common (DoublyLinkedList, Node)
                        └─ JsonStore / ActivityLog   lưu JSON (ghi nguyên tử), nhật ký JSONL
```

Bốn thư mục lõi của nhóm được **dùng nguyên**, không viết lại:

| Thư mục | Vai trò trong hệ thống |
|---|---|
| `Backend/Common` | `DanhSach` và `Node`: danh sách liên kết đôi, kiểm tra liên kết. Mỗi giỏ hàng là một danh sách. |
| `Backend/ProductManager` | Thêm / xóa / sửa sản phẩm trong giỏ; kiểm tra dữ liệu vào. |
| `Backend/ProductSearch` | Tìm theo mã, tìm theo tên; in bảng, duyệt xuôi và ngược. |
| `Backend/CartProcessing` | Tính tiền, **sắp xếp** giỏ. |
| `Backend/Integration` | `CartController`: một "cửa" ghép bốn module trên; kèm bản chạy dòng lệnh. |

Lớp `Backend/Api` chỉ thêm những thứ lõi chưa có: HTTP, JSON, băm mật khẩu, phiên đăng nhập, đơn hàng, mã giảm giá, báo cáo, nhật ký.
Giao diện gọi API cùng một địa chỉ với trang web nên không cần cấu hình CORS.

**Công nghệ:** C++17, CMake, OpenSSL (băm mật khẩu), nlohmann/json (kèm sẵn), HTTP/1.1 tự viết (mỗi kết nối một luồng, tối đa 256 kết nối), JavaScript ES modules không cần bước build, SVG cho biểu đồ.

---

## 5. Mã giảm giá

Mã giảm **theo phần trăm** (1–90 %). Mỗi mã có các điều kiện sau:

* **Đơn hàng tối thiểu:** tạm tính chưa đạt thì báo rõ khách cần mua thêm bao nhiêu.
* **Giới hạn số lượt dùng** (0 là không giới hạn).
* **Mỗi khách chỉ dùng 1 lần** (có thể tắt cho từng mã).
* Có thể **bật/tắt**; không có ngày hết hạn (muốn ngừng thì tắt mã).

Nguyên tắc thiết kế:

* **Máy chủ quyết định số tiền giảm.** Giao diện chỉ hiển thị. Khi đặt hàng, máy chủ kiểm tra lại mã và tính lại tiền trên giỏ thật, cùng lúc với việc trừ kho, nên khách có sửa dữ liệu gửi lên cũng vô tác dụng.
* **Lượt dùng không có bộ đếm riêng:** được đếm từ các đơn chưa hủy có dùng mã, nên **hủy đơn tự trả lại lượt** và số liệu không bao giờ bị lệch.
* Đơn lưu thêm `subtotal` (tạm tính), `couponCode`, `couponPercent`, `discount` và `total` (số khách phải trả). Doanh thu trong báo cáo là số tiền **sau giảm giá**.
* Mã không xóa được nếu đã có đơn dùng; hãy tắt mã thay vì xóa.

**Mã mẫu để demo** (đăng nhập `khach@shoppro.vn`, giỏ mẫu 3.536.000 ₫):

| Mã | Giảm | Điều kiện | Để demo |
|---|---|---|---|
| `WELCOME10` | 10 % | đơn từ 100.000 ₫, mỗi khách 1 lần | trường hợp cơ bản |
| `GIAM5` | 5 % | đơn từ 200.000 ₫, dùng nhiều lần | giỏ dưới 200.000 ₫ sẽ báo "cần mua thêm" |
| `GIAM15` | 15 % | đơn từ 1.000.000 ₫, 50 lượt | còn lượt |
| `SALE20` | 20 % | đơn từ 2.000.000 ₫, 30 lượt | giảm nhiều |
| `FLASH30` | 30 % | 3 lượt | thông báo **hết lượt** |
| `TET2026` | 12 % | đã tắt | thông báo **đang tạm ngưng** |

---

## 6. Dữ liệu và lưu trữ

Toàn bộ dữ liệu nằm trong **một tệp JSON** (theo yêu cầu đề bài: không dùng MySQL/SQL), được nạp vào RAM khi khởi động và **ghi nguyên tử** sau mỗi thay đổi (ghi tệp tạm → fsync → đổi tên, có bản sao `.bak`).
Nhật ký hoạt động ghi vào tệp `activity-log.jsonl` cùng thư mục.

**Dữ liệu mẫu** (`Backend/data/seed.json`): 65 sản phẩm / 8 danh mục (64 đang bán, 1 đang ẩn), 16 tài khoản (1 Admin, 15 khách, trong đó 1 bị khóa),
64 đơn hàng đủ mọi trạng thái (18 đơn có dùng mã giảm giá), 6 mã giảm giá, 369 dòng nhật ký.
Mốc thời gian được **dịch theo ngày hiện tại** mỗi lần nạp seed nên biểu đồ luôn có dữ liệu gần đây. Tạo lại seed (cần Python 3): `python3 Backend/data/tools/generate_seed.py`.

Dữ liệu chạy thật nằm ở `Backend/data/runtime/` (không có trong ZIP). Dữ liệu cũ tạo trước khi có mã giảm giá vẫn dùng được: lần khởi động đầu sẽ tự bổ sung các mã mẫu.

---

## 7. REST API

Mọi đường dẫn bắt đầu bằng `/api`, dữ liệu là JSON, lỗi trả về dạng `{"error": "thông báo tiếng Việt"}`. Ba mức phân quyền: `Public`, `Customer`, `Admin` (token riêng, **token khách không dùng được cho API Admin**).

| Nhóm | Ví dụ đường dẫn |
|---|---|
| Công khai | `GET /api/products`, `/api/products/:id`, `/api/categories`, `/api/health` |
| Tài khoản khách | `POST /api/auth/register`, `/api/auth/login`, `PATCH /api/users/me` |
| Giỏ hàng | `GET/POST/PATCH/DELETE /api/cart…`, `/api/cart/sort`, `/api/cart/search`, `/api/cart/structure` |
| Mã giảm giá (khách) | `GET /api/coupons`, `POST /api/coupons/validate` |
| Đơn hàng (khách) | `POST /api/orders`, `GET /api/orders`, `POST /api/orders/:id/cancel` |
| Trợ lý mua sắm | `POST /api/ai/chat` |
| Quản trị | `/api/admin/dashboard`, `/reports/summary`, `/products`, `/categories`, `/coupons`, `/orders`, `/customers`, `/settings`, `/activity-logs` |

Danh sách đầy đủ kèm dạng dữ liệu, mã lỗi và ví dụ `curl`: **[docs/API.md](docs/API.md)**.

---

## 8. Bảo mật

* Mật khẩu băm **PBKDF2-HMAC-SHA256** (600.000 vòng, muối ngẫu nhiên riêng từng người); so sánh thời gian hằng định.
* Phiên đăng nhập dùng token ngẫu nhiên (hết hạn sau 8 giờ, thu hồi khi đăng xuất hoặc khi tài khoản bị khóa); Admin và khách có đường đăng nhập và token tách biệt.
* Giới hạn tần suất đăng nhập/đăng ký; kiểm tra đầu vào; chặn tràn số ở giá, tồn kho, tổng tiền; ghi tệp nguyên tử.
* Số tiền giảm giá luôn do máy chủ tính (xem mục 5).
* Giao diện **escape mọi dữ liệu người dùng** (đã thử chèn `<script>`, `<img onerror>` vào tên, ghi chú, mô tả mã giảm giá); tiêu đề CSP chặn script nội tuyến.
* Xuất CSV chống chèn công thức Excel.

---

## 9. Đã kiểm tra những gì (và chưa kiểm tra gì)

**Đã chạy thật trên Linux (GCC 13) và Chrome thật:**

* Biên dịch sạch cảnh báo với `-Wall -Wextra -Wpedantic`; **toàn bộ 7 chương trình kiểm thử đạt (ctest)**, cả khi bật AddressSanitizer + UBSan.
  Trong đó `test_api` (kiểm thử REST đầu–cuối) có **636 kiểm tra** và `test_http` kiểm thử máy chủ qua socket thật.
* Chạy tự động bằng Chrome: đăng nhập, thêm giỏ, sắp xếp/tìm/duyệt ngược danh sách liên kết, thanh toán, hủy đơn, Admin nhận thông báo đơn mới, đổi trạng thái, từ chối chuyển trạng thái sai (HTTP 409),
  sửa giá/ẩn sản phẩm thì khách thấy ngay, khóa khách thì bị đăng xuất, thử XSS, quét màu (không có tông tím), không có chữ nhỏ hơn 14px, không tràn ngang trên màn hình 390px.
* **Mã giảm giá:** mã sai / hết lượt / tạm ngưng / chưa đủ đơn tối thiểu / đã dùng rồi; làm tròn tiền; sửa dữ liệu gửi lên vẫn bị máy chủ tính lại; hủy đơn trả lại lượt; ghi tệp lỗi thì mã không bị tính là đã dùng;
  nạp lại dữ liệu cũ không có mã vẫn chạy; Admin tạo/sửa/tắt/xóa; trên Chrome: áp mã ở giỏ và thanh toán, đặt đơn có mã, Admin thấy đơn và lượt dùng.
* Biên dịch và chạy được trên **Windows qua WSL (Ubuntu)** trên máy của thành viên nhóm.

**Chưa kiểm tra được** (xem thêm [Backend/README.md](Backend/README.md#giới-hạn-đã-biết)):

* Biên dịch và chạy **trực tiếp trên Windows (không qua WSL) và macOS**: mã đã có nhánh Winsock nhưng chưa thử; `test_http` chỉ chạy trên Linux/macOS.
* `Dockerfile` chưa được build thử vì môi trường phát triển không có Docker.
* **Chưa triển khai lên tên miền công khai**: cần tài khoản hosting. Hướng dẫn từng bước ở [docs/TRIEN_KHAI_WEB.md](docs/TRIEN_KHAI_WEB.md).
* Các ứng dụng React cũ của nhóm không nằm trong gói này; giao diện mới viết bằng HTML/CSS/JS thuần.

---

## 10. Kịch bản demo hai màn hình (5 phút)

1. Mở `/admin/` ở cửa sổ trái và `/shop/` ở cửa sổ phải; đăng nhập Admin và khách `khach@shoppro.vn` (nút **Điền sẵn**).
2. **Khách:** thêm vài sản phẩm, vào *Giỏ hàng*. Bấm **Sắp xếp giá giảm dần / tên A–Z** để thấy các dòng đổi chỗ. Gõ `chuot` vào ô tìm trong giỏ. Xem khung **"Giỏ hàng nhìn từ bên trong"** (node, `prev`/`next`, HEAD/TAIL) và bấm **Duyệt ngược**.
3. **Khách:** nhập `welcome10` (hoặc bấm một mã gợi ý) thì dòng *Giảm giá* và tổng mới hiện ra. Thử `FLASH30` (hết lượt), `TET2026` (tạm ngưng).
4. **Khách:** *Thanh toán*, đặt hàng.
5. **Admin:** trong vài giây có thông báo *"Có 1 đơn hàng mới"*; vào *Đơn hàng*, mở đơn vừa đặt để thấy tạm tính, mã và tiền giảm; bấm *Xác nhận*, *Bắt đầu giao*, *Hoàn thành*. Thử nhảy thẳng sang hoàn thành sẽ bị từ chối.
6. **Admin:** *Mã giảm giá* (lượt dùng vừa tăng; thử tạo mã mới, tắt một mã rồi quay lại cửa sổ khách nhập mã đó). Rồi *Tổng quan* (biểu đồ doanh thu), *Báo cáo* (chọn khoảng ngày, xuất CSV), *Sản phẩm* (sửa giá hoặc ẩn thì khách thấy ngay), *Khách hàng* (khóa tài khoản thì khách bị đăng xuất), *Nhật ký hoạt động* (mọi thao tác ở trên đều được ghi lại).

---

## 11. Cấu trúc thư mục

```
ShopPro/
├─ README.md                  Tổng quan dự án (tệp này)
├─ Dockerfile, .dockerignore  Đóng gói để triển khai lên web
├─ scripts/                   build / run / reset-data (.sh và .bat)
├─ docs/                      API.md, HUONG_DAN_CHAY_MAY_CHU.md, TRIEN_KHAI_WEB.md, báo cáo audit, nhật ký sửa lỗi DSA
├─ Backend/
│  ├─ README.md               Hướng dẫn biên dịch, chạy, kiểm thử chi tiết
│  ├─ CMakeLists.txt
│  ├─ Common/ ProductManager/ ProductSearch/ CartProcessing/ Integration/   Lõi DSA của nhóm
│  ├─ Api/                    REST API: http/, routes/, storage/, CartEngine, Coupons, Router
│  ├─ tests/                  Kiểm thử đơn vị, API và HTTP
│  ├─ data/                   seed.json, seed-activity.jsonl, tools/generate_seed.py
│  └─ third_party/nlohmann/   Thư viện JSON (kèm sẵn)
└─ Frontend/
   ├─ index.html              Trang chọn giao diện
   ├─ shop/                   Giao diện khách hàng (js/views/: trang chủ, sản phẩm, giỏ hàng, thanh toán, đơn hàng, tài khoản, trợ lý)
   ├─ admin/                  Giao diện quản trị (js/views/: tổng quan, sản phẩm, danh mục, mã giảm giá, đơn hàng, khách hàng, báo cáo, nhật ký, cài đặt)
   ├─ assets/                 CSS và thư viện JS dùng chung
   ├─ fonts/                  Inter, Lora (giấy phép OFL trong LICENSES.txt)
   └─ img/                    Ảnh sản phẩm SVG, biểu tượng
```

---

## 12. Tài liệu đi kèm

| Tài liệu | Dùng khi |
|---|---|
| [docs/HUONG_DAN_CHAY_MAY_CHU.md](docs/HUONG_DAN_CHAY_MAY_CHU.md) | Tự cài đặt và chạy máy chủ từ đầu (Windows + WSL), xử lý lỗi thường gặp |
| [Backend/README.md](Backend/README.md) | Biên dịch chi tiết, tham số dòng lệnh, kiểm thử, cấu trúc mã nguồn |
| [docs/API.md](docs/API.md) | Làm giao diện hoặc thử API: mọi đường dẫn, dữ liệu gửi và nhận, mã lỗi |
| [docs/TRIEN_KHAI_WEB.md](docs/TRIEN_KHAI_WEB.md) | Đưa hệ thống lên tên miền công khai (Docker, biến môi trường) |
| [docs/NHAT_KY_DIEU_CHINH_LOI_DSA.md](docs/NHAT_KY_DIEU_CHINH_LOI_DSA.md) | Các lỗi đã sửa trong lõi DSA và lý do |
| [docs/BAO_CAO_AUDIT_BACKEND.md](docs/BAO_CAO_AUDIT_BACKEND.md) | Báo cáo audit backend ban đầu |

---

## 13. Giới hạn đã biết

* Máy chủ là HTTP thuần, mỗi kết nối một luồng, tối đa 256 kết nối đồng thời: đủ cho demo và lớp học, không nhắm tới tải lớn. Muốn dùng HTTPS hãy đặt sau reverse proxy (Caddy, Nginx) hoặc nền tảng hosting.
* Dữ liệu nằm trong **một tệp JSON nạp hết vào RAM**; phù hợp tới vài nghìn đơn hàng, mỗi lần ghi là ghi lại cả tệp.
* Phiên đăng nhập nằm trong RAM: khởi động lại máy chủ thì mọi người phải đăng nhập lại (giỏ hàng và đơn hàng vẫn còn).
* Thanh toán chỉ ghi nhận phương thức (tiền mặt hoặc chuyển khoản), chưa tích hợp cổng thanh toán.
* Mã giảm giá chỉ giảm theo phần trăm, không có ngày hết hạn và không gắn riêng cho từng khách.
* Trợ lý mua sắm là bộ luật đơn giản (ngân sách, danh mục, từ khóa), không phải mô hình ngôn ngữ.
* Tài khoản demo và nút "Điền sẵn" ở trang đăng nhập chỉ nên dùng khi trình diễn.
