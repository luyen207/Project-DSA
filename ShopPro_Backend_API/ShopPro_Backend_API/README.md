# ShopPro – Hệ thống quản lý giỏ hàng mua sắm trực tuyến

ShopPro là một cửa hàng trực tuyến hoàn chỉnh gồm **máy chủ C++** (REST API), **giao diện khách hàng** và **giao diện quản trị**.
Lõi giỏ hàng dùng **danh sách liên kết đôi** do nhóm tự cài đặt cho môn *Cấu trúc dữ liệu & Giải thuật*;
toàn bộ phần còn lại (đăng nhập, đơn hàng, báo cáo, nhật ký…) được xây xung quanh lõi đó.

Một tiến trình duy nhất phục vụ cả hai giao diện và API, nên có thể mở **hai cửa sổ cùng lúc** để demo
(khách đặt hàng → Admin nhận thông báo ngay).

| Địa chỉ | Dành cho |
|---|---|
| `/` | Trang chọn giao diện |
| `/shop/` | Cửa hàng (khách hàng) |
| `/admin/` | Quản trị viên |
| `/api/...` | REST API (xem [docs/API.md](docs/API.md)) |

---

## 1. Chạy nhanh

Cần: trình biên dịch C++17 (GCC ≥ 9 / Clang ≥ 10 / MSVC 2019+), **CMake ≥ 3.18**, **OpenSSL** (thư viện phát triển).
Không cần Internet khi biên dịch (thư viện JSON đã kèm sẵn trong `Backend/third_party`).

```bash
# Linux / macOS
scripts/build.sh          # biên dịch (thêm --test để chạy luôn kiểm thử)
scripts/run.sh            # mở http://127.0.0.1:8080/
```

Nếu báo `Permission denied` (do giải nén làm mất quyền chạy), dùng `bash scripts/build.sh` và `bash scripts/run.sh`.

```bat
:: Windows (xem lưu ý trong Backend\README.md)
scripts\build.bat
scripts\run.bat
```

Mở trình duyệt tại **http://127.0.0.1:8080/** rồi chọn *Cửa hàng* hoặc *Quản trị viên*.
Hướng dẫn biên dịch chi tiết, tham số dòng lệnh và kiểm thử: **[Backend/README.md](Backend/README.md)**.

### Tài khoản demo (chỉ dùng để trình diễn)

| Vai trò | Email | Mật khẩu |
|---|---|---|
| Quản trị viên | `admin@shoppro.vn` | `Admin@12345` |
| Khách hàng | `khach@shoppro.vn` | `Demo@12345` |
| Khách hàng khác | `minhanh@example.com`, `quocbao@example.com`, `thuha@example.com`… | `Demo@12345` |
| Khách bị khóa (để demo) | `vankhoa@example.com` | `Demo@12345` |

Trang đăng nhập có nút **Điền sẵn** cho tài khoản demo. Khi đưa lên mạng thật, hãy xóa khung "Tài khoản demo"
trong `Frontend/shop/js/views/account.js` và `Frontend/admin/js/app.js`, rồi đổi mật khẩu Admin
(xem [docs/TRIEN_KHAI_WEB.md](docs/TRIEN_KHAI_WEB.md)).

---

## 2. Kịch bản demo hai màn hình (gợi ý 5 phút)

1. Mở `/admin/` ở cửa sổ trái và `/shop/` ở cửa sổ phải; đăng nhập Admin và khách `khach@shoppro.vn`.
2. **Khách:** thêm vài sản phẩm → vào *Giỏ hàng*. Bấm **Sắp xếp giá giảm dần / tên A–Z**: các dòng trượt về vị trí mới.
   Gõ `chuot` vào ô tìm trong giỏ (không dấu vẫn ra "Chuột"). Xem khung **"Giỏ hàng nhìn từ bên trong"**:
   mỗi sản phẩm là một node có `prev`/`next`, nhãn HEAD/TAIL, bấm **Duyệt ngược** để đi từ tail về head,
   huy hiệu *Liên kết hợp lệ* là kết quả của hàm `kiemTraLienKet()`.
3. **Khách:** *Thanh toán* → đặt hàng.
4. **Admin:** trong vài giây xuất hiện thông báo *"Có 1 đơn hàng mới"*; vào *Đơn hàng* → *Xác nhận* → *Bắt đầu giao* → *Hoàn thành*.
   Thử chuyển sai quy trình (ví dụ nhảy thẳng sang hoàn thành) sẽ bị từ chối.
5. **Admin:** *Tổng quan* (biểu đồ doanh thu 14 ngày, trạng thái đơn, hàng sắp hết), *Báo cáo* (chọn khoảng ngày, xuất CSV),
   *Sản phẩm* (sửa giá, ẩn/hiện – khách thấy thay đổi ngay), *Khách hàng* (khóa tài khoản → khách bị đăng xuất),
   *Nhật ký hoạt động* (mọi thao tác ở trên đều được ghi lại).

---

## 3. Kiến trúc

```
Trình duyệt ──HTTP──►  shoppro_api (C++17, một tiến trình)
 /shop/  (khách)        ├─ HttpServer      máy chủ HTTP/1.1 + phục vụ tệp tĩnh Frontend/
 /admin/ (quản trị)     ├─ Router          định tuyến, phân quyền Public / Customer / Admin
                        ├─ routes/*        nghiệp vụ: auth, giỏ hàng, đơn hàng, admin
                        ├─ CartEngine      giỏ hàng của từng khách = một CartController
                        │     └─ CartController ─► ProductManager · ProductSearch · CartProcessing
                        │                               └──────────► Common (DoublyLinkedList, Node)
                        └─ JsonStore / ActivityLog   lưu JSON (ghi nguyên tử), nhật ký JSONL
```

Bốn thư mục lõi của nhóm được **dùng nguyên** (không viết lại):

| Thư mục | Vai trò trong hệ thống |
|---|---|
| `Backend/Common` | `DanhSach`/`Node`: danh sách liên kết đôi, kiểm tra liên kết. Mỗi giỏ hàng là một danh sách. |
| `Backend/ProductManager` | Thêm / xóa / sửa sản phẩm trong giỏ; thêm trùng thì **cộng dồn** số lượng. |
| `Backend/ProductSearch` | Tìm theo mã, tìm theo tên (ô *tìm trong giỏ*), in bảng/duyệt xuôi–ngược (khung cấu trúc node). |
| `Backend/CartProcessing` | Tính tổng tiền, **sắp xếp** giỏ (giá tăng/giảm, tên, mã). |

Lớp `Backend/Api` chỉ thêm những thứ lõi chưa có: HTTP, JSON, mật khẩu băm, phiên đăng nhập, đơn hàng, báo cáo, nhật ký.
Giỏ hàng được nạp vào danh sách liên kết khi cần và **ghi xuống tệp sau mỗi thao tác**, nên khởi động lại máy chủ không mất giỏ.
Các lỗi đã sửa trong lõi DSA trước khi tích hợp nằm ở [docs/NHAT_KY_DIEU_CHINH_LOI_DSA.md](docs/NHAT_KY_DIEU_CHINH_LOI_DSA.md);
báo cáo audit ban đầu ở [docs/BAO_CAO_AUDIT_BACKEND.md](docs/BAO_CAO_AUDIT_BACKEND.md).

---

## 4. Tính năng

**Khách hàng:** đăng ký / đăng nhập, danh mục, tìm kiếm không phân biệt dấu, lọc giá và tồn kho, sắp xếp, phân trang,
chi tiết sản phẩm, giỏ hàng (sắp xếp / tìm / xem cấu trúc node), thanh toán (COD hoặc chuyển khoản), theo dõi tiến trình đơn,
hủy đơn khi còn *Chờ xác nhận*, hồ sơ cá nhân, **trợ lý mua sắm** (hiểu "dưới 2 triệu", "500k", "so sánh…"; dùng luật trên dữ liệu cửa hàng, không gọi dịch vụ bên ngoài).

**Quản trị:** tổng quan có biểu đồ SVG (kèm chế độ xem bảng), quản lý sản phẩm (thêm/sửa/nhập kho/ẩn/xóa), danh mục,
đơn hàng theo máy trạng thái *chờ xác nhận → xác nhận → giao hàng → hoàn thành* (hủy trả hàng về kho), khách hàng (khóa/mở khóa),
báo cáo theo khoảng ngày + xuất CSV, nhật ký hoạt động có lọc, cài đặt cửa hàng, thông báo đơn mới.

**Dữ liệu mẫu (`Backend/data/seed.json`):** 65 sản phẩm / 8 danh mục (64 đang bán, 1 đang ẩn), 16 tài khoản (1 Admin, 15 khách – 1 bị khóa),
64 đơn hàng đủ mọi trạng thái, 363 dòng nhật ký. Mốc thời gian được **dịch theo ngày hiện tại** mỗi lần nạp seed nên biểu đồ luôn có dữ liệu gần đây.
Ảnh sản phẩm là SVG vẽ sẵn trong `Frontend/img/products` (chạy được khi không có Internet).

---

## 5. Bảo mật đã làm

* Mật khẩu băm **PBKDF2-HMAC-SHA256** (600.000 vòng, muối ngẫu nhiên riêng từng người); so sánh thời gian hằng định.
* Phiên đăng nhập dùng token ngẫu nhiên (hết hạn, thu hồi khi đăng xuất hoặc khi bị khóa); Admin và khách có đường đăng nhập và token tách biệt.
* Giới hạn tần suất đăng nhập/đăng ký; kiểm tra đầu vào, chặn tràn số ở giá/tồn kho/tổng tiền; ghi tệp nguyên tử (tmp → fsync → rename, có `.bak`).
* Giao diện escape mọi dữ liệu người dùng (đã thử chèn `<script>`/`<img onerror>` vào tên, ghi chú); tiêu đề CSP chặn script nội tuyến.
* Giao diện CSV chống chèn công thức Excel.

---

## 6. Đã kiểm tra những gì (và chưa kiểm tra gì)

Đã chạy thật trong môi trường Linux (GCC 13, Chrome):

* Biên dịch sạch cảnh báo với `-Wall -Wextra -Wpedantic`; **toàn bộ ctest đạt**, cả khi bật AddressSanitizer + UBSan
  (7 chương trình kiểm thử, gồm `test_api` – kiểm thử REST đầu–cuối – và `test_http` – kiểm thử qua socket thật).
* Chạy tự động bằng Chrome thật: đăng nhập, thêm giỏ, sắp xếp/tìm/duyệt ngược danh sách liên kết, thanh toán, hủy đơn,
  Admin nhận thông báo đơn mới, đổi trạng thái, từ chối chuyển trạng thái sai (HTTP 409), sửa giá/ẩn sản phẩm → khách thấy ngay,
  khóa khách → bị đăng xuất, thử XSS, quét màu (không có tông tím), không có chữ nhỏ hơn 14px, không tràn ngang trên màn hình 390px.

**Chưa kiểm tra được** (xem thêm [Backend/README.md](Backend/README.md#giới-hạn-đã-biết)):

* Biên dịch và chạy trên **Windows/macOS** (mã đã có nhánh Winsock nhưng chưa thử); `test_http` chỉ chạy trên Linux/macOS.
* `Dockerfile` chưa được build thử vì môi trường phát triển không có Docker daemon.
* **Chưa triển khai lên tên miền công khai** – cần tài khoản hosting của bạn. Hướng dẫn từng bước ở [docs/TRIEN_KHAI_WEB.md](docs/TRIEN_KHAI_WEB.md).
* Các ứng dụng React cũ của nhóm không nằm trong gói này; giao diện mới viết bằng HTML/CSS/JS thuần, không cần bước build.

---

## 7. Cấu trúc thư mục

```
ShopPro/
├─ README.md                  Tổng quan dự án (tệp này)
├─ Dockerfile, .dockerignore  Đóng gói để triển khai lên web
├─ scripts/                   build / run / reset-data (.sh và .bat)
├─ docs/                      API.md, TRIEN_KHAI_WEB.md, báo cáo audit, nhật ký sửa lỗi DSA
├─ Backend/
│  ├─ README.md               Hướng dẫn biên dịch, chạy, kiểm thử
│  ├─ CMakeLists.txt
│  ├─ Common/ ProductManager/ ProductSearch/ CartProcessing/ Integration/   Lõi DSA của nhóm
│  ├─ Api/                    REST API: http/, routes/, storage/, CartEngine, Router
│  ├─ tests/                  Kiểm thử đơn vị, API và HTTP
│  ├─ data/                   seed.json, seed-activity.jsonl, tools/generate_seed.py
│  └─ third_party/nlohmann/   Thư viện JSON (kèm sẵn)
└─ Frontend/
   ├─ index.html              Trang chọn giao diện
   ├─ shop/                   Giao diện khách hàng
   ├─ admin/                  Giao diện quản trị
   ├─ assets/                 CSS và thư viện JS dùng chung
   ├─ fonts/                  Inter, Lora (giấy phép OFL trong LICENSES.txt)
   └─ img/                    Ảnh sản phẩm SVG, biểu tượng
```

Dữ liệu chạy thật được tạo ở `Backend/data/runtime/` (không nằm trong ZIP). Muốn quay về dữ liệu mẫu: `scripts/reset-data.sh`.
