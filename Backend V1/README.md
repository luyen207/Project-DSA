# ShopPro Backend – Hướng dẫn biên dịch, chạy và kiểm thử

Backend gồm hai tầng, biên dịch chung bằng **một** `CMakeLists.txt`:

* **Lõi DSA của nhóm:** `Common`, `ProductManager`, `ProductSearch`, `CartProcessing`, `Integration` (thư viện tĩnh `shoppro_core`).
* **Tầng REST API + máy chủ web:** `Api/` (thư viện `shoppro_api_lib`, `shoppro_http`, chương trình `shoppro_api`).

Tổng quan dự án và demo: xem [../README.md](../README.md). Danh sách API: [../docs/API.md](../docs/API.md).

## 1. Yêu cầu

| Thành phần | Phiên bản |
|---|---|
| Trình biên dịch C++17 | GCC ≥ 9, Clang ≥ 10 hoặc MSVC 2019+ |
| CMake | ≥ 3.18 |
| OpenSSL (thư viện phát triển) | 1.1.1 hoặc 3.x |
| Internet | **Không cần** (nlohmann/json 3.12.0 đã kèm trong `third_party/`) |

Cài OpenSSL:

```bash
# Ubuntu / Debian / WSL
sudo apt install build-essential cmake libssl-dev
# macOS (Homebrew)
brew install cmake openssl
#   nếu CMake không tìm thấy OpenSSL:  -DOPENSSL_ROOT_DIR="$(brew --prefix openssl)"
# Fedora
sudo dnf install gcc-c++ cmake openssl-devel
```

## 2. Biên dịch

Chạy từ thư mục gốc dự án (`ShopPro/`):

```bash
cmake -S Backend -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Hoặc dùng `scripts/build.sh` (Linux/macOS) – tương đương hai lệnh trên.

Tùy chọn CMake:

| Tùy chọn | Mặc định | Ý nghĩa |
|---|---|---|
| `-DCMAKE_BUILD_TYPE=Release` | Release | Kiểu build (Debug để gỡ lỗi) |
| `-DSHOPPRO_BUILD_TESTS=ON/OFF` | ON | Có biên dịch các chương trình kiểm thử không |
| `-DSHOPPRO_SANITIZE=ON` | OFF | Bật AddressSanitizer + UBSan cho mọi target (GCC/Clang) |

Sản phẩm sinh ra trong `build/`:

| Tệp | Mô tả |
|---|---|
| `shoppro_api` | **Máy chủ web + REST API** (chương trình chính) |
| `shoppro_cli` | Bản dòng lệnh gốc của nhóm (menu quản lý giỏ hàng trên console) |
| `test_*`, `demo_kiem_tra_lien_ket` | Kiểm thử và chương trình minh họa |

### Windows

Mã đã có nhánh Winsock (`ws2_32`) và cờ `/W4 /utf-8` cho MSVC, **nhưng chưa được kiểm thử trên Windows** (môi trường phát triển là Linux).
Nếu gặp lỗi, gửi lại thông báo lỗi để chỉnh. Có hai cách chuẩn bị OpenSSL:

*Cách 1 – Visual Studio + vcpkg*

```bat
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install openssl:x64-windows
set VCPKG_ROOT=C:\vcpkg
scripts\build.bat
scripts\run.bat
```

*Cách 2 – MSYS2 (UCRT64)*

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-openssl
cmake -S Backend -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/shoppro_api.exe
```

`test_http` dùng socket POSIX nên **không** được biên dịch trên Windows; các kiểm thử còn lại vẫn chạy.

## 3. Chạy máy chủ

```bash
build/shoppro_api                       # http://127.0.0.1:8080/
build/shoppro_api --port 9000
build/shoppro_api --host 0.0.0.0        # cho máy khác trong mạng LAN truy cập
```

Chương trình tự tìm thư mục dự án (đi lên từ thư mục hiện tại hoặc từ vị trí tệp chạy cho đến khi thấy `Backend/data/seed.json`),
nên có thể chạy từ thư mục gốc hoặc từ `build/`. Lần chạy đầu, dữ liệu mẫu được sao sang `Backend/data/runtime/shoppro.json`.

| Tham số | Biến môi trường | Mặc định |
|---|---|---|
| `--port N` | `PORT` | `8080` |
| `--host ADDR` | `HOST` | `127.0.0.1` |
| `--data FILE` | `SHOPPRO_DATA` | `Backend/data/runtime/shoppro.json` |
| `--seed FILE` | `SHOPPRO_SEED` | `Backend/data/seed.json` |
| `--web DIR` | `SHOPPRO_WEB` | `Frontend/` |
| `--trust-proxy` | `TRUST_PROXY=1` | tắt – chỉ bật khi chạy sau reverse proxy |
| `--quiet` | – | không in nhật ký từng yêu cầu ra màn hình (mặc định có in) |
| `--create-admin EMAIL TÊN` | `SHOPPRO_ADMIN_PASSWORD` | tạo thêm tài khoản Admin rồi thoát |

Tạo Admin mới (mật khẩu 8–128 ký tự, đặt qua biến môi trường để không lộ trên dòng lệnh):

```bash
SHOPPRO_ADMIN_PASSWORD='MatKhauManh#2026' build/shoppro_api --create-admin giaovien@truong.edu.vn "Giáo viên"
```

Quay về dữ liệu mẫu: dừng máy chủ rồi chạy `scripts/reset-data.sh` (hoặc xóa thư mục `Backend/data/runtime/`).

Dừng máy chủ bằng `Ctrl+C`. Dữ liệu được ghi xuống tệp ngay sau mỗi thay đổi nên không cần thao tác lưu.

## 4. Kiểm thử

```bash
ctest --test-dir build --output-on-failure
```

| Chương trình | Nội dung kiểm tra |
|---|---|
| `test_doubly_linked_list` | Danh sách liên kết đôi: khởi tạo, chèn, xóa, kiểm tra con trỏ prev/next |
| `test_product_manager` | Quản lý sản phẩm trong giỏ: thêm, cộng dồn, sửa, xóa, kiểm tra dữ liệu |
| `test_product_search` | Tìm theo mã và theo tên |
| `test_cart_calculator` | Tính tổng tiền, sắp xếp và các trường hợp biên |
| `test_cart_controller` | Façade tích hợp bốn module |
| `test_api` | Kiểm thử REST đầu–cuối (đăng nhập, phân quyền, giỏ hàng, đơn hàng, quy trình trạng thái, báo cáo, nhật ký, đầu vào xấu) |
| `test_http` | Máy chủ HTTP qua socket thật (phân tích yêu cầu, giới hạn kích thước, phục vụ tệp tĩnh, chống path traversal) – **chỉ Linux/macOS** |

Kiểm thử cùng bộ phát hiện lỗi bộ nhớ:

```bash
cmake -S Backend -B build-asan -DCMAKE_BUILD_TYPE=Debug -DSHOPPRO_SANITIZE=ON
cmake --build build-asan --parallel
ctest --test-dir build-asan --output-on-failure
```

`demo_kiem_tra_lien_ket` là chương trình minh họa (không nằm trong ctest): cố tình làm hỏng một con trỏ để cho thấy `kiemTraLienKet()` phát hiện được.

## 5. Cấu trúc mã nguồn

```
Backend/
├─ Common/            Node, DoublyLinkedList – danh sách liên kết đôi
├─ ProductManager/    thêm / sửa / xóa / cộng dồn sản phẩm, kiểm tra dữ liệu
├─ ProductSearch/     tìm theo mã/tên, in bảng, duyệt xuôi/ngược
├─ CartProcessing/    tính tiền, sắp xếp
├─ Integration/       CartController (façade) + main.cpp của bản dòng lệnh
├─ Api/
│  ├─ main.cpp        điểm vào của shoppro_api, đọc tham số
│  ├─ Support.*       tiện ích: băm mật khẩu, thời gian, văn bản, kiểm tra số
│  ├─ ApiContext.h, Router.cpp   ngữ cảnh dùng chung, định tuyến, phân quyền
│  ├─ CartEngine.*    giỏ hàng từng khách (CartController + đồng bộ với kho + khôi phục khi ghi lỗi)
│  ├─ http/           máy chủ HTTP/1.1 (đa luồng), phục vụ tệp tĩnh
│  ├─ storage/        JsonStore (ghi nguyên tử), ActivityLog (JSONL)
│  └─ routes/         Public, Auth, Cart, Order, AdminCatalog, AdminOps, Presenters
├─ tests/             kiểm thử
├─ data/              seed.json, seed-activity.jsonl, tools/generate_seed.py
└─ third_party/       nlohmann/json
```

Tạo lại dữ liệu mẫu (cần Python 3): `python3 Backend/data/tools/generate_seed.py`.

## 6. Cách giỏ hàng dùng danh sách liên kết đôi

1. Mỗi khách có một `CartController` (trong `CartEngine`), bên trong là một `DanhSach` (danh sách liên kết đôi).
2. `POST /api/cart/items` → `ProductManager::themSanPham` (trùng mã thì cộng dồn số lượng).
3. `POST /api/cart/sort` → `ProductSort` (đổi chỗ dữ liệu giữa các node; liên kết được giữ nguyên và kiểm tra lại).
4. `GET /api/cart/search?q=` → `ProductSearch::timTheoTen` (tên đã bỏ dấu nên gõ không dấu vẫn tìm được).
5. `GET /api/cart/structure?direction=forward|backward` → duyệt từ head hoặc tail, trả về từng node cùng `prev`/`next` và kết quả `kiemTraLienKet()`.
6. Sau **mỗi** thao tác làm thay đổi giỏ, nội dung được ghi vào `data["carts"][userId]`. Nếu ghi đĩa lỗi, danh sách trong RAM được dựng lại
   từ dữ liệu cũ (`CartEngine::rebuild`) để RAM và tệp không lệch nhau.
7. Trước khi trả giỏ, `syncWithCatalog` loại sản phẩm đã bị xóa/ẩn và cập nhật tên, giá mới nhất từ kho.

## Giới hạn đã biết

* **Windows/macOS chưa kiểm thử** (xem mục Windows). Đường dẫn tìm thư mục dự án dùng `/proc/self/exe` trên Linux; trên hệ khác có dùng thư mục hiện tại.
* Máy chủ là HTTP thuần, mỗi kết nối một luồng, tối đa 256 kết nối đồng thời – đủ cho demo/lớp học, không nhắm tới tải lớn.
  Muốn dùng HTTPS hãy đặt sau reverse proxy (Caddy, Nginx) hoặc nền tảng hosting (xem [../docs/TRIEN_KHAI_WEB.md](../docs/TRIEN_KHAI_WEB.md)).
* Dữ liệu lưu trong **một tệp JSON** nạp hết vào RAM (theo yêu cầu không dùng cơ sở dữ liệu SQL). Phù hợp tới vài nghìn đơn hàng;
  mỗi lần ghi là ghi lại cả tệp.
* Phiên đăng nhập nằm trong RAM: khởi động lại máy chủ thì mọi người phải đăng nhập lại (giỏ hàng và đơn hàng vẫn còn).
* Trợ lý mua sắm là bộ luật đơn giản (ngân sách, danh mục, từ khóa), không phải mô hình ngôn ngữ.
* Thanh toán chỉ ghi nhận phương thức (COD/chuyển khoản), chưa tích hợp cổng thanh toán.
* Tài khoản demo và khung "Điền sẵn" ở trang đăng nhập chỉ nên dùng khi trình diễn.
