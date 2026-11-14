# ShopPro Backend — Báo cáo AUDIT & kế hoạch tái cấu trúc

Ngày audit: 14/11/2026 · Phạm vi: toàn bộ thư mục `Backend/` (không tính các thư mục `build*`) · Giai đoạn: chỉ phân tích, **không sửa file nào trong thư mục của bạn**.

## Cách kiểm tra (để biết nhận định nào đã được chứng minh)

- Đọc từng dòng **toàn bộ mã nguồn `.cpp/.h`** (Common, ProductManager, ProductSearch, CartProcessing, Integration), 2 file CMake, `README.md`, `BACKEND_GUIDE.md`, `data/seed.json` và `tests/test_rest_api.cpp` (không chỉ dựa vào tên file). Các test DSA (5 file + demo) mình **biên dịch và chạy** nhưng không đọc từng dòng; các README/TestCases/TestReport trong thư mục module không đọc.
- Biên dịch **bản sao** trong thư mục tạm bằng g++ 13, C++17, `-Wall -Wextra -Wpedantic`, AddressSanitizer + UBSan: **0 cảnh báo**.
- Chạy 6 bộ test: 53/53, 54/54, 26/26, 27/27, 17/17 (lõi DSA) và 31/31 (REST) — **đều pass**.
- Chạy thử `shoppro_api` thật (health, 401, JSON lỗi, CORS, kết nối treo) và 3 chương trình thăm dò nhỏ để kiểm chứng lỗi nghi ngờ.
- Mỗi lỗi ở Phần D ghi rõ: **[đã xác nhận]** (chạy thật, có bằng chứng) hay **[suy luận từ code]**.
- Chưa làm: xem Frontend (ngoài phạm vi giai đoạn này); chưa chạy trên MSYS2 UCRT64 của bạn (CMakeCache cho thấy bạn build bằng `C:/msys64/ucrt64/bin/c++.exe` + Ninja; các file `.exe` trong `build-*` cho thấy bản đó đã biên dịch được).

> **Phát hiện quan trọng nhất về hiện trạng:** Backend **đã có sẵn** HTTP Server (tự viết bằng socket), REST API, JSON File Storage và băm mật khẩu PBKDF2. Nhiều thứ đề bài coi là "cần xây" thì thực tế đã có. Thứ **thật sự thiếu** là Activity Log, và có một số lỗi cần sửa (Phần D).

---

## PHẦN A: Tổng quan Backend hiện tại

### A.1 Cấu trúc thực tế

```
Backend/
├── CMakeLists.txt            (gốc: chỉ add_subdirectory(Integration))
├── Common/                   Node, DoublyLinkedList            (TV1)
├── ProductManager/           ProductManager, ProductValidation (TV2)
├── ProductSearch/            ProductSearch, ProductDisplay     (TV3)
├── CartProcessing/           CartCalculator, ProductSort       (TV4)
├── Integration/
│   ├── CartController.*      façade "một cửa" cho giỏ hàng DSA
│   ├── main.cpp              CLI (shoppro_cli)
│   ├── api_main.cpp          điểm vào của REST API (shoppro_api)
│   ├── HttpServer.*          HTTP server tự viết bằng socket
│   ├── RestApi.*             toàn bộ routes + auth + lưu JSON (1.287 dòng)
│   └── CMakeLists.txt        build toàn bộ
├── data/seed.json            dữ liệu mẫu (runtime: data/shoppro.json, đã .gitignore)
├── tests/                    5 test DSA + test_rest_api + demo_kiem_tra_lien_ket
└── build, build-alt, build-local, build-root   (4 thư mục build dư thừa)
```

### A.2 Hai tầng độc lập

| Tầng | File | Dùng để |
|---|---|---|
| **Lõi DSA** (danh sách liên kết đôi) | Common, ProductManager, ProductSearch, CartProcessing, CartController | CLI `shoppro_cli` và các test |
| **REST API** | HttpServer, RestApi, api_main (+ nlohmann/json, OpenSSL) | Frontend Customer/Admin |

**Điểm then chốt:** `RestApi.cpp` chỉ include thư viện chuẩn, nlohmann/json và OpenSSL — **không gọi bất kỳ module DSA nào**. Giỏ hàng của REST là mảng JSON `[{productId, quantity}]` theo từng user; `CartController`/`DoublyLinkedList` chỉ phục vụ CLI và test. Nghĩa là câu "kết nối API với Services hiện tại" hiện **chưa xảy ra** (xem Phần I, giai đoạn 4).

### A.3 Luồng chạy khi có request

```
Trình duyệt ──HTTP──► HttpServer.cpp (accept → readRequest → handleConnection)
                          │  parse request line/headers/body, giới hạn 16KB/1MB, CORS
                          ▼
                     RestApi::handle()  (khóa mutex)
                          │  parseTarget → xác thực Bearer token → kiểm tra role → route
                          ▼
                     Json data_ (RAM) ──persist()──► data/shoppro.json (ghi .tmp rồi đổi tên)
```

### A.4 Build & dependency

- CMake ≥ 3.18, C++17. `FetchContent` kéo `nlohmann/json` (ghim theo commit), `find_package(OpenSSL REQUIRED)`, Windows link thêm `ws2_32`.
- Target: `shoppro_core` (static lib), `shoppro_cli`, `shoppro_api`, 5 test DSA, `test_rest_api`, `demo_kiem_tra_lien_ket`; `ctest` chạy 6 test.
- Lệnh đúng (README): `cmake -S . -B build` → `cmake --build build` → `ctest --test-dir build`.

---

## PHẦN B: Đánh giá cấu trúc OOP

| Khía cạnh | Đánh giá |
|---|---|
| **Lõi DSA** | Tốt cho mức sinh viên: `DanhSach` có destructor (RAII), cấm copy (`= delete`); `CartController` đóng gói `DanhSach` bằng composition và là cửa duy nhất; validation tập trung ở `ProductValidation`; lỗi trả qua enum `MaKetQua`; namespace rõ ràng (`ProductSearch`, `CartCalculator`…). |
| **Điểm yếu lõi DSA** | `DanhSach`/`Node` có thành viên public (đóng gói yếu); `khoiTaoDanhSach()` gọi trên danh sách đang có node sẽ **rò bộ nhớ** (chỉ reset con trỏ, không giải phóng) — nay đã thừa vì có constructor; sắp xếp bubble sort O(n²) bằng cách đổi **dữ liệu** giữa các node (đúng ý đồ, ghi rõ trong tài liệu). |
| **Tầng REST** | Chỉ có **1 class** `RestApi` với 2 hàm public. `handle()` là một chuỗi ~36 nhánh `if` dài ~740 dòng (dòng 545–1285): route, xác thực, nghiệp vụ, đọc/ghi dữ liệu trộn trong cùng một hàm. Không có Controller/Service/Repository; `nlohmann::json` đóng vai "model" (không có class `Product`, `Order`, `User`). Đây là phong cách thủ tục, **không phải OOP**. |
| **Điểm tốt tầng REST** | Hàm trợ giúp tách trong namespace ẩn danh (`publicProduct`, `totalFor`, `requiredString`…); mẫu "sao chép dữ liệu → sửa → ghi file → hoán đổi vào RAM" nhất quán nên lỗi ghi file không làm hỏng dữ liệu đang chạy. |
| **Thống nhất mô hình** | Hai tầng có hai mô hình khác nhau: DSA dùng `double` cho giá và chuẩn hóa tên chỉ A–Z (sai với tiếng Việt có dấu); REST dùng `int64` đồng và `lower()` hỗ trợ UTF-8 tiếng Việt. Đây là giới hạn nhóm đã ghi nhận (#6, #7 trong BACKEND_GUIDE.md). |

**Kết luận B:** OOP đạt ở tầng lõi, chưa đạt ở tầng REST. Cách sửa an toàn là tách `RestApi.cpp` theo chức năng **sau khi** đã có đủ test (Phần I, giai đoạn 3).

---

## PHẦN C: Bảng đánh giá các chức năng

Ký hiệu: ✅ Hoàn thành · 🟡 Hoàn thành một phần · ❌ Chưa triển khai · 🐞 Có lỗi cần khắc phục. "Test" = đã có trong `test_rest_api` (31 kiểm tra).

### User Management

| Chức năng | Trạng thái | Ghi chú |
|---|---|---|
| Đăng ký | ✅ (kèm rủi ro **D3**) | `POST /api/auth/register`; validate tên/email/SĐT/mật khẩu ≥ 8; băm PBKDF2; email trùng → 409. Test có. |
| Đăng nhập | ✅ (kèm **D5**) | Token ngẫu nhiên 256-bit, sống 8 giờ, lưu trong RAM (khởi động lại phải đăng nhập lại). Admin có endpoint riêng. |
| Đăng xuất | ✅ | Xóa token khỏi bộ nhớ. |
| Quản lý thông tin | 🟡 | `PATCH /api/users/me` sửa tên/SĐT/địa chỉ. Không đổi được mật khẩu/email; không xóa trắng được địa chỉ (bị coi là rỗng → 400). |
| Phân quyền Admin/Customer | ✅ | Role được kiểm ở **cả** session lẫn dữ liệu; customer gọi API admin → 403, admin gọi API customer → 403 (test có); role **không bao giờ** đọc từ body; tài khoản Admin chỉ tạo được bằng CLI `--create-admin`. |
| Khóa / mở khóa tài khoản | ✅ | `PATCH /api/admin/customers/{id}/status`; khóa thì thu hồi ngay mọi session; khách bị khóa đăng nhập → 403 (test có). Không khóa được Admin. |

### Product Management

| Chức năng | Trạng thái | Ghi chú |
|---|---|---|
| Thêm sản phẩm | ✅ | `POST /api/admin/products`; giá 1–10¹² đồng, tồn 0–1.000.000, danh mục phải tồn tại. |
| Xóa sản phẩm | ✅ | Bị chặn (409) nếu đã nằm trong đơn → gợi ý ẩn sản phẩm (test có). |
| Cập nhật sản phẩm | 🟡 (**D6**) | `PUT` + `PATCH …/stock` + `PATCH …/visibility`; riêng `featured`/`bestSelling` gửi lên bị **bỏ qua** [đã xác nhận]. |
| Xem danh sách | ✅ | Phân trang (limit ≤ 100), lọc tên/danh mục/giá, sắp xếp `price-asc`, `price-desc`, `newest`; tìm kiếm không phân biệt hoa/thường và dấu tiếng Việt cơ bản (test có). |
| Xem chi tiết | ✅ | `GET /api/products/{id}` (ẩn sản phẩm đã ẩn), có `/related`. |

### Cart Management

| Chức năng | Trạng thái | Ghi chú |
|---|---|---|
| Thêm vào giỏ | ✅ | Kiểm tra tồn kho và cộng dồn số lượng (test có). |
| Xóa sản phẩm khỏi giỏ | 🐞 **D1** | `DELETE /api/cart/items/{id}` — use-after-free [đã xác nhận bằng ASan]. |
| Cập nhật số lượng | 🐞 **D1** | `PATCH /api/cart/items/{id}` — cùng lỗi [đã xác nhận]. |
| Tính tổng tiền | ✅ | Server tự tính từ giá trong kho dữ liệu, chặn tràn số (test có). |
| Kiểm tra sản phẩm/số lượng hợp lệ | ✅ | 1 ≤ số lượng ≤ min(tồn kho, 1.000.000); sản phẩm ẩn/không tồn tại → 404. |
| Dùng lõi DSA (`CartController`) | 🟡 | Chỉ CLI dùng; REST không dùng (Phần A.2). |

### Order Management

| Chức năng | Trạng thái | Ghi chú |
|---|---|---|
| Tạo đơn hàng | ✅ | `POST /api/orders`: đối chiếu với giỏ lưu trên server (lệch → 409), **tự lấy giá từ server**, kiểm tồn, trừ tồn kho, xóa giỏ (test có). |
| Lưu thông tin đơn | ✅ | Mã `SP-` + 12 hex ngẫu nhiên, lưu snapshot tên/ảnh/đơn giá từng dòng. |
| Chi tiết đơn | ✅ | `GET /api/orders/{id}` — chỉ chủ đơn xem được, người khác → 404 (test có). |
| Theo dõi trạng thái | ✅ | `pending → confirmed → shipping → completed`; hủy được từ `pending`/`confirmed`; hủy thì hoàn tồn kho (test có). |
| Admin cập nhật trạng thái | ✅ | `PATCH /api/admin/orders/{id}/status`, chặn chuyển trạng thái sai (409), `completed`/`cancelled` là cuối (test có). |
| Customer xem lịch sử | ✅ | `GET /api/orders`, mới nhất trước. |
| Ghi chú | — | Không có thanh toán (đúng như README nêu). Customer chưa có route tự hủy đơn (không nằm trong yêu cầu). |

### Activity Log

| Chức năng | Trạng thái | Ghi chú |
|---|---|---|
| Hoạt động đăng ký | ❌ | Tìm trong `RestApi.cpp`/`HttpServer.cpp`: **không có** cơ chế log nghiệp vụ nào. |
| Đăng nhập | ❌ | Như trên. |
| Thao tác giỏ hàng | ❌ | Như trên. |
| Đặt hàng | ❌ | Như trên. |
| Hoạt động quản trị | ❌ | Như trên. |

---

## PHẦN D: Các lỗi và rủi ro cần khắc phục

Sắp theo mức độ. Số dòng theo `Integration/RestApi.cpp` trừ khi ghi khác.

| # | Mức | Vấn đề | Bằng chứng |
|---|---|---|---|
| **D1** | **Cao** | **Use-after-free ở `PATCH /api/cart/items/{id}` (dòng 784) và `DELETE /api/cart/items/{id}` (dòng 796).** `const Json& user = requireCustomer()` là tham chiếu vào `data_["users"]`. Sau `data_ = std::move(next)` vùng nhớ cũ bị giải phóng, nhưng câu `return` vẫn gọi `user.value("id","")`. Đây chính là thao tác "đổi số lượng" và "xóa khỏi giỏ" mà Frontend sẽ dùng khi demo. Các route khác không dính lỗi vì đã sao chép `userId` ra chuỗi riêng trước khi ghi (dòng 724, 754). | **[đã xác nhận]** ASan báo `heap-use-after-free` đúng tại dòng 784 và 796 khi gọi thử. `test_rest_api` vẫn 31/31 vì **không có test nào gọi 2 route này**, và `test_rest_api`/`shoppro_api` không được bật sanitizer (option `SHOPPRO_SANITIZE` chỉ áp dụng cho `shoppro_core`). |
| **D2** | **Cao (khi demo 2 giao diện)** | **Server đơn luồng** (`HttpServer.cpp` dòng 244–261: accept → xử lý → đóng, tuần tự). Một kết nối mở sẵn nhưng không gửi dữ liệu sẽ chặn mọi client khác tới 10 giây (timeout tính theo từng lần `recv`, client gửi nhỏ giọt có thể giữ lâu hơn). Customer và Admin cùng gọi một Backend nên có thể bị treo ngắt quãng. | **[đã xác nhận]** mở 1 socket rỗng, `GET /api/health` từ client khác bị timeout sau 4 giây; server tự hồi phục sau khi socket treo hết hạn. Hai ý còn lại — client gửi nhỏ giọt giữ lâu hơn 10 giây, và trình duyệt có mở kết nối nhàn rỗi hay không — là **[suy luận]**. |
| **D3** | **Trung bình** | **Đăng ký bằng email của khách có sẵn trong dữ liệu mẫu sẽ "nhận" hồ sơ đó.** `customers` trong seed (cus-1001…1006) có hồ sơ nhưng chưa có tài khoản. Dòng 610–618 tái sử dụng hồ sơ khi trùng email, không xác minh email; sau đó `GET /api/orders` (dòng 808–815) trả đơn cũ kèm địa chỉ và SĐT của người đó. Đồng thời dòng 629 xóa giỏ hàng cũ của hồ sơ. | **[đã xác nhận]** đăng ký `minhanh@example.com` → nhận `userId = cus-1001` và thấy đơn `SP-2401081`, `SP-2401021` cùng địa chỉ/SĐT. Với dữ liệu demo thì ảnh hưởng thấp, nhưng là lỗ hổng phân quyền thật. |
| **D4** | **Trung bình** | **Chưa có Activity Log** (Phần C). | **[đã xác nhận]** không có trong code. |
| **D5** | Thấp–TB | Không giới hạn số lần đăng nhập sai. Thời gian phản hồi khác nhau giữa email **có** tài khoản (phải tính PBKDF2 ~hàng trăm ms) và email **không có** (trả ngay) nên dò được email; `register` trả 409 cũng lộ email. | **[suy luận từ code]** dòng 641–652. |
| **D6** | Thấp | `PUT /api/admin/products/{id}` bỏ qua `featured`/`bestSelling` trong body (dòng 1027–1028 ghi đè bằng giá trị cũ). Nếu Admin UI có công tắc "Nổi bật/Bán chạy" thì sẽ không lưu được. | **[đã xác nhận]** gửi `featured=true` → phản hồi vẫn `false`. |
| **D7** | Thấp | `(page - 1) * limit` (dòng 686) tràn số nguyên có dấu khi `page` rất lớn → hành vi không xác định. Cần giới hạn `page`. | **[đã xác nhận]** UBSan: `signed integer overflow` với `page=9223372036854775807`. |
| **D8** | Thấp | `/api/ai/chat` với số ngân sách quá dài làm `std::stod` ném lỗi (dòng 910) → trả **500** thay vì 400. | **[đã xác nhận]** 400 chữ số + " triệu" → 500. |
| **D9** | Thấp | Số vòng lặp PBKDF2 lưu theo từng user (`passwordIterations`, dòng 534/627) nhưng **không bao giờ được đọc lại**; `validPassword` luôn dùng hằng số 210.000. Nếu sau này tăng hằng số, mọi mật khẩu cũ sẽ sai. Ngoài ra 210.000 vòng thấp hơn mức OWASP hiện khuyến nghị cho PBKDF2-HMAC-SHA256 (khoảng 600.000 — bạn nên đối chiếu lại tài liệu OWASP hiện hành). | **[đã xác nhận]** grep: chỉ có chỗ ghi, không có chỗ đọc. |
| **D10** | Thấp | Chạy `--create-admin` **trong lúc server đang chạy**: hai tiến trình cùng ghi một file, không có khóa → server ghi đè làm mất Admin vừa tạo. README đã hướng dẫn tạo Admin trước khi chạy server nên chỉ cần ghi chú cảnh báo. | **[suy luận từ code]** |
| **D11** | Thấp | `persist()` ghi `.tmp` rồi đổi tên (tốt) nhưng không `fsync` trên Linux/macOS và không giữ bản `.bak`; file JSON hỏng → server từ chối khởi động (báo lỗi rõ), không tự phục hồi. | **[đọc code]** dòng 465–508. |
| **D12** | Thấp | `HttpServer.cpp`: `send()` không dùng `MSG_NOSIGNAL`/bỏ qua `SIGPIPE` — trên Linux/macOS, client ngắt kết nối giữa chừng có thể làm chết tiến trình. Windows (môi trường của bạn) không bị. | **[chưa tái hiện được]** thử 1 lần, server vẫn sống. Chỉ nên coi là gia cố. |
| **D13** | Thấp | Tài liệu/CMake: dòng 3 của `Integration/CMakeLists.txt` hướng dẫn `cmake -S Integration …` nhưng file này không có `project()` nên không phải thư mục gốc hợp lệ (README ghi đúng là `cmake -S . -B build`). 4 thư mục build dư (~320 file). Mã 405 được khai báo nhưng không bao giờ trả (route/method lạ → 404). | **[đọc code]** |

**Không phải lỗi (đã kiểm tra, an toàn):** giá không lấy từ Frontend (luôn đọc từ dữ liệu server); không có SQL; mật khẩu không bao giờ lưu dạng thuần (chỉ `passwordSalt` + `passwordHash`; test kiểm tra file lưu); khách không tự đổi được role; khách không đọc được giỏ/đơn của người khác (định danh lấy từ token); body ≤ 1 MB, header ≤ 16 KB, từ chối `Transfer-Encoding`; CORS chỉ cho `localhost`/`127.0.0.1` cổng 5173/5174 (origin lạ không nhận header cho phép); mật khẩu tối đa 256 ký tự (chặn DoS băm); so sánh hash dùng `CRYPTO_memcmp` (thời gian không đổi).

---

## PHẦN E: Khả năng tích hợp HTTP Server và REST API

**Đã có và chạy được.** Không cần "bổ sung HTTP Server" từ đầu.

| Hạng mục | Hiện trạng |
|---|---|
| Xây HTTP Server | Tự viết bằng socket (Winsock/POSIX) trong `HttpServer.cpp`; mặc định `127.0.0.1:8080`; tham số `--host --port --data`. Không keep-alive (`Connection: close`), không TLS. |
| Định nghĩa routes | Chuỗi `if` so khớp `method` + `path`/`parts` trong `RestApi::handle()`. Chạy được nhưng dài và khó bảo trì. |
| Nhận JSON | `parseObject()`: body rỗng → `{}`; không phải object → 400; sai cú pháp → 400. |
| Trả JSON | `HttpResponse{status, json}` → `Content-Type: application/json; charset=utf-8`, `Cache-Control: no-store`, `nosniff`. |
| HTTP status | 200, 201, 204, 400, 401, 403, 404, 409, 413, 500 (405 khai báo nhưng không dùng). |
| Xử lý lỗi | `ApiError{status, message}` → `{ "error": "..." }`; lỗi JSON lạ → 400; lỗi hệ thống → 500 với thông điệp chung (không lộ chi tiết nội bộ). Lỗi ghi file không làm hỏng dữ liệu RAM. |
| Xác thực | `Authorization: Bearer <token>`, token 64 ký tự hex từ `RAND_bytes`, lưu trong `std::map` RAM, hết hạn 8 giờ (chỉ xóa khi được dùng lại → map có thể phình chậm). |
| Phân quyền | `requireAdmin()` / `requireCustomer()` kiểm tra role trong session **và** trong dữ liệu, trạng thái khóa; chủ sở hữu giỏ/đơn lấy từ token. |
| Kết nối với Services hiện tại | **Không có** (Phần A.2). |

### Nên dùng thư viện nào?

| Lựa chọn | Ưu | Nhược | Khuyến nghị |
|---|---|---|---|
| **Giữ server hiện tại** + thêm `std::thread` cho mỗi kết nối | Đổi rất ít code: `RestApi::handle()` đã có `std::mutex` bảo vệ `data_`/`sessions_`. Sửa được D2. | Vẫn tự lo parser HTTP. | ✅ **Làm trước** (giai đoạn 2). |
| **cpp-httplib** (1 file header) | Router, thread pool, keep-alive có sẵn; dễ cài. | Phải chuyển `HttpServer.cpp`. | Nên cân nhắc nếu muốn bỏ code HTTP tự viết. |
| **Crow** | Router/middleware đẹp, đúng hướng đề bài. | Kéo thêm Asio, cấu hình nặng hơn trên MSYS2 UCRT64; phải viết lại phần lớn `handle()`. | Chưa cần thiết. |

**Kết luận E:** tích hợp REST **đã đạt**; việc cần làm là ổn định (đa luồng, sửa D1) rồi mới tính chuyện đổi thư viện.

---

## PHẦN F: Kiến trúc JSON File Storage

### F.1 Hiện trạng

Một file `data/shoppro.json` với các khóa cấp cao: `users`, `customers`, `products`, `categories`, `orders`, `carts`, `settings`. Lần chạy đầu tự sao chép từ `data/seed.json`. File runtime được `.gitignore`.

### F.2 Quan hệ giữa các đối tượng

```
categories.id ◄── products.categoryId
products.id   ◄── orders.items[].productId     (đơn lưu snapshot tên/ảnh/đơn giá)
customers.id  ==  users.id (với customer)       ◄── orders.customerId
users.id      ◄── carts[userId]                 (mảng {productId, quantity})
```

### F.3 So với danh sách file bạn dự kiến

| File dự kiến | Đề xuất |
|---|---|
| users / products / carts / orders | **Giữ chung một file** `shoppro.json`. Lý do: một lần đặt hàng phải sửa đồng thời `products` (trừ tồn), `orders`, `carts`, `customers`, `users`. Tách 5 file sẽ mất tính nguyên tử (ghi file 1 xong, file 2 lỗi → dữ liệu lệch). |
| **activity_logs.json** | **Tách riêng** và đổi sang dạng **JSON Lines** (`activity_logs.jsonl`, mỗi sự kiện một dòng, chỉ nối thêm cuối file). Nhật ký tăng nhanh; ghi lại cả file mỗi request thì chậm dần. |

### F.4 Cách đọc/ghi/cập nhật (đã có)

- Khởi động: đọc `shoppro.json` vào RAM; chưa có thì lấy từ seed; kiểm tra đủ 7 khóa.
- Mỗi thay đổi: **sao chép** `data_` → sửa bản sao → `persist()` (ghi `shoppro.json.tmp` rồi `MoveFileExW`/`rename` thay thế) → chỉ khi thành công mới hoán đổi vào RAM. Lỗi giữa chừng → RAM và file đều giữ bản cũ.
- Đồng thời: `std::mutex` trong `handle()` tuần tự hóa mọi request (đủ an toàn trong cùng một tiến trình).
- Sau khi khởi động lại: dữ liệu giữ nguyên (test "JSON storage survives an API restart" có); chỉ token đăng nhập mất.

### F.5 Điểm cần bổ sung

1. Activity log dạng JSONL (Phần I, giai đoạn 1).
2. Giữ 1 bản sao lưu `shoppro.json.bak` trước mỗi lần thay thế, và `fsync` trên Linux/macOS (D11).
3. Khóa file (hoặc ghi chú rõ) để tránh hai tiến trình cùng ghi (D10).
4. Nếu dữ liệu lớn dần: mỗi request đang sao chép **toàn bộ** JSON (O(n)) — chấp nhận được cho demo, cần xem lại nếu có hàng nghìn đơn.

### F.6 Mật khẩu

PBKDF2-HMAC-SHA256, salt 16 byte ngẫu nhiên (`RAND_bytes`), so sánh bằng `CRYPTO_memcmp`, **không lưu mật khẩu thuần** ở bất kỳ đâu (seed không chứa tài khoản; test đọc file kiểm tra). Cần xử lý D9 (đọc lại số vòng lặp đã lưu để có thể nâng cấp sau này).

---

## PHẦN G: Kiến trúc Backend đề xuất

Nguyên tắc: **không di chuyển/đổi tên** 4 module của TV1–TV4 và `CartController`/CLI. Chỉ chia nhỏ `Integration/RestApi.cpp`, và chỉ làm khi đã có đủ test (Phần J).

```
Backend/
├── Common/ ProductManager/ ProductSearch/ CartProcessing/   (giữ nguyên)
├── Integration/
│   ├── CartController.*  main.cpp                           (giữ nguyên — CLI)
│   ├── api_main.cpp
│   ├── http/      HttpServer.{h,cpp}          ← "server"
│   ├── api/       Router (RestApi.cpp còn lại: định tuyến + middleware xác thực)
│   │              AuthHandlers / ProductHandlers / CartHandlers /
│   │              OrderHandlers / AdminHandlers              ← "controllers"
│   ├── storage/   JsonStore.{h,cpp}  (đọc, persist, sao lưu)  ← "repositories"
│   │              ActivityLog.{h,cpp} (JSONL, chỉ nối thêm)
│   ├── security/  PasswordHasher.{h,cpp}  SessionStore.{h,cpp} ← "middleware/auth"
│   └── util/      Json (integer, requiredString…), Text (lower, trim, urlDecode), Time
├── data/ seed.json   (runtime, gitignore: shoppro.json, activity_logs.jsonl)
├── tests/
└── CMakeLists.txt
```

Đối chiếu với cấu trúc tham khảo của bạn: `routes`+`controllers` → `api/`; `services` → logic nghiệp vụ (đơn hàng, giỏ) tách thành hàm trong handler, chỉ lập class Service khi thật sự cần; `repositories` → `storage/`; `middleware` → `security/`; `utils` → `util/`. Không cần thư mục `include/`/`config/` riêng ở giai đoạn này.

---

## PHẦN H: Danh sách REST API cần xây

Tất cả nằm dưới `/api`. Lỗi luôn có dạng `{ "error": "<thông báo>" }`. Cột "Có sẵn" cho biết route đã tồn tại (✅) hay cần thêm (➕).

### H.1 Authentication

| Method | Endpoint | Quyền | Request body | Response (thành công) | Status | Có sẵn |
|---|---|---|---|---|---|---|
| POST | `/auth/register` | Công khai | `name, email, phone, password` | `{user, token}` | 201 / 400 / 403 (khóa) / 409 (trùng email) | ✅ (sửa D3) |
| POST | `/auth/login` | Công khai | `email, password` | `{user, token}` | 200 / 400 / 401 / 403 | ✅ |
| POST | `/admin/auth/login` | Công khai | `email, password` | `{user(role=admin), token}` | 200 / 401 / 403 | ✅ |
| GET | `/auth/me`, `/admin/auth/me` | Customer / Admin | — | thông tin user | 200 / 401 / 403 | ✅ |
| POST | `/auth/logout`, `/admin/auth/logout` | Customer / Admin | — | `{message}` | 200 / 401 | ✅ |

### H.2 Customer

| Method | Endpoint | Quyền | Body | Response | Status | Có sẵn |
|---|---|---|---|---|---|---|
| PATCH | `/users/me` | Customer | `name?, phone?, address?` | user | 200 / 400 | ✅ |
| PATCH | `/users/me/password` | Customer | `currentPassword, newPassword` | `{message}` | 200 / 400 / 401 | ➕ (tùy chọn — chỉ làm nếu nhóm xác nhận nằm trong phạm vi) |
| POST | `/ai/chat` | Customer | `message` | `{answer, products[], comparison}` | 200 / 400 (sửa D8) | ✅ |

### H.3 Product

| Method | Endpoint | Quyền | Body / Query | Response | Status | Có sẵn |
|---|---|---|---|---|---|---|
| GET | `/products` | Công khai | `page, limit(≤100), search, category, minPrice, maxPrice, sort` | `{items[], page, limit, total}` | 200 / 400 | ✅ (giới hạn `page`, D7) |
| GET | `/products/{id}` | Công khai | — | product | 200 / 404 | ✅ |
| GET | `/products/{id}/related` | Công khai | — | product[] (≤ 4) | 200 / 404 | ✅ |
| GET | `/categories` | Công khai | — | tên danh mục[] | 200 | ✅ |
| GET | `/admin/products` | Admin | — | product[] (kèm ẩn) | 200 | ✅ |
| POST | `/admin/products` | Admin | `name, categoryId, price, stock, description?, image?, visible?, featured?, bestSelling?` | product | 201 / 400 | ✅ |
| PUT | `/admin/products/{id}` | Admin | như trên | product | 200 / 400 / 404 | ✅ (sửa D6) |
| PATCH | `/admin/products/{id}/stock` | Admin | `stock` | product | 200 / 400 / 404 | ✅ |
| PATCH | `/admin/products/{id}/visibility` | Admin | `visible` | product | 200 / 400 / 404 | ✅ |
| DELETE | `/admin/products/{id}` | Admin | — | — | 204 / 404 / 409 | ✅ |
| GET/POST/PUT/DELETE | `/admin/categories[/{id}]` | Admin | `name, description?` | category(+productCount) | 200/201/204 / 400 / 404 / 409 | ✅ |

### H.4 Cart (Customer)

| Method | Endpoint | Body | Response | Status | Nghiệp vụ | Có sẵn |
|---|---|---|---|---|---|---|
| GET | `/cart` | — | `{items[], total}` | 200 | Tổng tính ở server | ✅ |
| POST | `/cart/items` | `productId, quantity` | giỏ | 200 / 400 / 404 / 409 | Kiểm tồn, cộng dồn | ✅ |
| PATCH | `/cart/items/{productId}` | `quantity` | giỏ | 200 / 400 / 404 | Đặt lại số lượng | ✅ (**sửa D1**) |
| DELETE | `/cart/items/{productId}` | — | giỏ | 200 / 400 | Xóa 1 dòng | ✅ (**sửa D1**) |
| DELETE | `/cart` | — | — | 204 | Làm trống giỏ | ✅ |

### H.5 Order

| Method | Endpoint | Quyền | Body | Response | Status | Nghiệp vụ | Có sẵn |
|---|---|---|---|---|---|---|---|
| POST | `/orders` | Customer | `customerName, phone, shippingAddress, note?, items[{productId, quantity}]` | order | 201 / 400 / 409 | So khớp giỏ, **giá lấy từ server**, trừ tồn, xóa giỏ | ✅ |
| GET | `/orders` | Customer | — | order[] | 200 | Chỉ đơn của mình | ✅ |
| GET | `/orders/{id}` | Customer | — | order | 200 / 404 | Chỉ chủ đơn | ✅ |
| GET | `/admin/orders` | Admin | — | order[] | 200 | Tất cả đơn | ✅ |
| PATCH | `/admin/orders/{id}/status` | Admin | `status` | order | 200 / 400 / 404 / 409 | Kiểm tra chuyển trạng thái; hủy → hoàn tồn | ✅ |

### H.6 Admin — khách hàng, báo cáo, cài đặt

| Method | Endpoint | Body / Query | Status | Có sẵn |
|---|---|---|---|---|
| GET | `/admin/customers` | — | 200 | ✅ |
| GET | `/admin/customers/{id}/orders` | — | 200 | ✅ |
| PATCH | `/admin/customers/{id}/status` | `status: active\|locked` | 200 / 400 / 404 / 409 | ✅ |
| GET | `/admin/dashboard` | — | 200 | ✅ |
| GET | `/admin/reports/summary` | `from, to` (YYYY-MM-DD) | 200 / 400 | ✅ |
| GET / PUT | `/admin/settings` | `storeName, supportEmail, supportPhone, lowStockThreshold, currency` | 200 / 400 | ✅ |

### H.7 Activity Log — **cần xây mới**

| Method | Endpoint | Quyền | Query | Response | Status |
|---|---|---|---|---|---|
| GET | `/admin/activity-logs` | Admin | `type, actorId, from, to, page, limit` | `{items[], page, limit, total}` | 200 / 400 / 401 / 403 |

Mỗi sự kiện: `{id, timestamp, actorId, actorRole, action, targetType, targetId, ip?, detail}`. Các `action` cần ghi: `auth.register`, `auth.login`, `auth.login_failed`, `auth.logout`, `cart.add`, `cart.update`, `cart.remove`, `cart.clear`, `order.create`, `order.status_changed`, `admin.product.*`, `admin.category.*`, `admin.customer.lock/unlock`, `admin.settings.update`. **Không bao giờ ghi mật khẩu, hash hay token vào log.**

---

## PHẦN I: Kế hoạch tái cấu trúc theo từng giai đoạn

Mỗi giai đoạn kết thúc khi `ctest` xanh 100% và ASan/UBSan sạch.

| GĐ | Nội dung | Kết quả cần đạt |
|---|---|---|
| **0 — Sửa lỗi chặn demo** | Sửa **D1** (sao chép `userId` trước khi ghi, đúng mẫu dòng 724/754) và **thêm test** cho `PATCH`/`DELETE /cart/items/{id}` + `DELETE /cart`. Bật `-fsanitize=address,undefined` cho `shoppro_api`/`test_rest_api`. Sửa **D7** (giới hạn `page`), **D8** (trả 400). | Test mới chạy sạch dưới ASan; không còn lỗi bộ nhớ. |
| **1 — Activity Log** | Thêm `ActivityLog` (JSONL, chỉ nối thêm, có mutex), gắn vào các điểm trong H.7, thêm `GET /admin/activity-logs`, thêm vào `.gitignore`. | Mọi hành động ở H.7 để lại đúng 1 dòng log; log không chứa bí mật. |
| **2 — Ổn định demo** | Thread-per-connection (sửa **D2**); sửa **D3** (không tái dùng hồ sơ có sẵn khi đăng ký); giảm lộ email/giới hạn đăng nhập sai (**D5**); quyết định **D6**; bỏ qua `SIGPIPE` (D12); đọc lại số vòng lặp PBKDF2 đã lưu (**D9**). | 2 giao diện dùng đồng thời không bị treo; test đăng ký email seed → không thấy đơn cũ. |
| **3 — Tách mã** | Theo Phần G: tách `util/`, `storage/JsonStore`, `security/PasswordHasher`, rồi các `*Handlers`. Mỗi bước tách là 1 lần commit riêng, **không đổi hành vi**. | Số test không giảm; `RestApi.cpp` chỉ còn router. |
| **4 — Quyết định về lõi DSA (nhóm cần chốt)** | Nếu yêu cầu môn học coi danh sách liên kết đôi là lõi giỏ hàng: cho REST dùng `CartController` (mỗi user một giỏ trong RAM, đồng bộ với JSON) hoặc ghi rõ trong báo cáo rằng CLI minh họa DSA còn REST dùng JSON. Cần thống nhất kiểu tiền (`double` ↔ `int64`). | Quyết định bằng văn bản; nếu chọn tích hợp thì có test cho cả hai đường. |
| **5 — Tích hợp Frontend** | Đối chiếu từng route với các service TypeScript của Customer/Admin; chạy kịch bản demo (Phần J.6); dọn 4 thư mục build dư, sửa comment CMake (**D13**). | Demo trọn vẹn Customer ↔ Admin trên cùng Backend. |

---

## PHẦN J: Checklist kiểm thử

Ký hiệu: ☑ đã có trong `test_rest_api` · ☐ cần viết thêm.

### J.1 Xác thực & phân quyền
- ☑ Admin API không token → 401 · ☑ customer gọi API admin → 403 · ☑ admin gọi API customer → 403
- ☑ Đăng nhập bị khóa → 403 · ☑ khóa tài khoản thu hồi token
- ☐ Token hết hạn → 401 · ☐ Token sai/rỗng/`Bearer` thiếu · ☐ Gửi `role` trong body đăng ký không có tác dụng
- ☐ Đăng ký bằng email của khách seed **không** thấy đơn cũ (D3) · ☐ Đăng nhập sai nhiều lần (D5)
- ☑ File lưu chỉ có `passwordHash`, không có mật khẩu thuần

### J.2 Sản phẩm
- ☑ Danh sách, tìm kiếm tiếng Việt, chi tiết · ☑ xóa sản phẩm đã có trong đơn → 409
- ☐ Tạo/sửa/xóa thành công, `PATCH stock`, `PATCH visibility` · ☐ giá/tồn ngoài biên → 400
- ☐ `page`/`limit` không hợp lệ hoặc rất lớn (D7) · ☐ `PUT` với `featured` (D6)
- ☐ CRUD danh mục + xóa danh mục còn sản phẩm → 409

### J.3 Giỏ hàng
- ☑ Thêm vào giỏ + tổng tiền · ☑ checkout xóa giỏ
- ☐ **`PATCH /cart/items/{id}`** (D1) · ☐ **`DELETE /cart/items/{id}`** (D1) · ☐ `DELETE /cart`
- ☐ Số lượng 0/âm/vượt tồn · ☐ Sản phẩm bị ẩn/xóa khi đang ở trong giỏ

### J.4 Đơn hàng
- ☑ Giỏ lệch → 409 · ☑ tạo đơn, trừ tồn, snapshot giá · ☑ khách khác không xem được đơn
- ☑ Chuyển trạng thái đúng/sai, hủy hoàn tồn, trạng thái cuối
- ☐ Hai khách mua tranh tồn kho cuối cùng · ☐ Gửi giá giả trong body (phải bị bỏ qua)

### J.5 Hạ tầng
- ☑ Dữ liệu còn sau khi khởi động lại
- ☐ Body > 1 MB → 413 · ☐ Header > 16 KB · ☐ JSON sai cú pháp → 400 · ☐ CORS origin lạ
- ☐ Kết nối treo không làm chậm client khác (D2) · ☐ File JSON hỏng khi khởi động · ☐ Hết dung lượng khi ghi
- ☐ Activity log: mỗi hành động ở H.7 → 1 dòng, không chứa bí mật

### J.6 Kịch bản demo thủ công (2 giao diện)
1. `shoppro_api --create-admin …` → chạy server. 2. Customer đăng ký → xem sản phẩm → thêm giỏ → **đổi số lượng** → **xóa 1 dòng** → đặt hàng.
3. Admin đăng nhập → thấy đơn mới → xác nhận → giao → hoàn tất. 4. Customer thấy trạng thái đổi.
5. Admin hủy một đơn khác → tồn kho hoàn lại. 6. Admin khóa khách → khách bị đăng xuất. 7. Tắt/bật Backend → dữ liệu còn nguyên.

---

## KẾT LUẬN

1. **Backend hiện tại có tiếp tục dùng được không?** **Có.** Lõi DSA ổn (177/177 test pass dưới ASan/UBSan, 0 cảnh báo), tầng REST chạy được (31/31, server thật đáp ứng đúng). Nhưng phải sửa D1 trước khi demo.
2. **Nên giữ nguyên:** `Common/`, `ProductManager/`, `ProductSearch/`, `CartProcessing/`, `CartController` và CLI; logic đọc request của `HttpServer.cpp`; mẫu ghi file nguyên tử; luồng PBKDF2; kiểm tra role; nghiệp vụ đơn hàng (giá từ server, kiểm tồn, hoàn tồn khi hủy).
3. **Bắt buộc sửa:** D1 (lỗi bộ nhớ ở giỏ hàng). **Nên sửa trước demo:** D2 (server đơn luồng), D3 (nhận hồ sơ khách seed), D4 (Activity Log — nếu nằm trong yêu cầu), D6/D7/D8 (nhỏ).
4. **Có cần bổ sung HTTP Server?** **Không** — đã có (tự viết bằng socket). Cần nâng cấp đa luồng; chuyển sang Crow/cpp-httplib là tùy chọn, chưa cần.
5. **Có cần bổ sung JSON Storage?** **Không cần dựng mới** — đã có `shoppro.json`. Chỉ thêm `activity_logs.jsonl` và bản sao lưu. Không nên tách 5 file cho dữ liệu chính vì sẽ mất tính nguyên tử khi đặt hàng.
6. **Đã sẵn sàng tích hợp Frontend chưa?** **Gần sẵn sàng.** Hầu hết route đã có; còn D1 ảnh hưởng đúng thao tác giỏ hàng, và mình **chưa đối chiếu hợp đồng JSON với Frontend** (ngoài phạm vi giai đoạn này).
7. **Việc tiếp theo:** thực hiện **Giai đoạn 0** (sửa D1 + test hồi quy + bật sanitizer), sau đó **Giai đoạn 1** (Activity Log). Mình chưa chỉnh sửa gì — khi bạn đồng ý sẽ làm từng giai đoạn một.
