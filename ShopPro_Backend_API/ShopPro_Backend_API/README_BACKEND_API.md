# ShopPro – Gói dành cho thành viên làm Backend / API

Gói này là dự án ShopPro **không có thư mục `Frontend/`**. Phần Backend (C++) vẫn biên dịch, chạy và kiểm thử đầy đủ.
Máy chủ khởi động bình thường khi thiếu `Frontend/` (dòng "Frontend : (khong tim thay)" là bình thường); các trang web chỉ mở được khi có Frontend, còn **API vẫn dùng được**.

## Chạy nhanh

Xem `Backend/README.md` (Linux/macOS/WSL):

```bash
bash scripts/build.sh --test     # biên dịch + chạy 7 chương trình kiểm thử
bash scripts/run.sh              # máy chủ ở http://127.0.0.1:8080/
curl http://127.0.0.1:8080/api/health
```

`Dockerfile` và `docs/TRIEN_KHAI_WEB.md` chỉ dùng được với **bản đầy đủ** (có `Frontend/`); trong gói này đừng chạy `docker build`.

Thử API bằng curl: xem mục cuối của `docs/API.md`. Tài khoản demo: khách `khach@shoppro.vn` / `Demo@12345`, quản trị `admin@shoppro.vn` / `Admin@12345`.

## Bản đồ file trong `Backend/`

| Thư mục | Vai trò | File chính |
|---|---|---|
| `Common/` | Danh sách liên kết đôi dùng chung | `Node.*`, `DoublyLinkedList.*` |
| `ProductManager/` | Thêm / sửa / xóa / cộng dồn sản phẩm, kiểm tra dữ liệu | `ProductManager.*`, `ProductValidation.*` |
| `ProductSearch/` | Tìm theo mã / tên, hiển thị | `ProductSearch.*`, `ProductDisplay.*` |
| `CartProcessing/` | Tính tiền, sắp xếp | `CartCalculator.*`, `ProductSort.*` |
| `Integration/` | Ghép bốn module thành một façade + bản dòng lệnh | `CartController.*`, `main.cpp` |
| `Api/` | **Máy chủ web + REST API** | xem bên dưới |
| `tests/` | Kiểm thử | `test_*.cpp` (7 chương trình trong ctest) + `demo_kiem_tra_lien_ket.cpp` |
| `data/` | Dữ liệu mẫu | `seed.json`, `seed-activity.jsonl`, `tools/generate_seed.py` |
| `third_party/` | Thư viện JSON kèm sẵn | `nlohmann/json.hpp` |
| `CMakeLists.txt` | Cấu hình biên dịch chung | |

### Bên trong `Backend/Api/`

| File | Vai trò |
|---|---|
| `main.cpp` | Điểm vào, đọc tham số dòng lệnh và biến môi trường |
| `http/HttpServer.*`, `HttpTypes.h` | Máy chủ HTTP/1.1, phục vụ file tĩnh |
| `Router.cpp`, `ApiContext.h` | Định tuyến và phân quyền (Public / Customer / Admin) |
| `Support.*` | Băm mật khẩu, thời gian, kiểm tra dữ liệu vào |
| `CartEngine.*` | Giỏ hàng từng khách (dùng `CartController` + danh sách liên kết đôi) |
| `storage/JsonStore.*` | Ghi file JSON an toàn (ghi nguyên tử, có bản sao `.bak`) |
| `storage/ActivityLog.*` | Nhật ký hoạt động (JSONL) |
| `routes/PublicRoutes.cpp` | Sản phẩm, danh mục, cửa hàng, health |
| `routes/AuthRoutes.cpp` | Đăng ký, đăng nhập, đăng xuất, hồ sơ |
| `routes/CartRoutes.cpp` | Giỏ hàng: thêm, sửa, xóa, sắp xếp, tìm, xem cấu trúc |
| `routes/OrderRoutes.cpp` | Đặt hàng, xem và hủy đơn của khách |
| `routes/AdminCatalogRoutes.cpp` | Admin: sản phẩm và danh mục |
| `routes/AdminOpsRoutes.cpp` | Admin: dashboard, báo cáo, đơn hàng, khách hàng, cài đặt, nhật ký |
| `routes/Presenters.*` | Đổi dữ liệu nội bộ thành JSON trả về |

## Khi sửa xong

Chỉ gửi lại các file/thư mục bạn đã sửa trong `Backend/` (kèm tên file). Chạy `bash scripts/build.sh --test` trước khi gửi để chắc là vẫn đạt 7/7.
