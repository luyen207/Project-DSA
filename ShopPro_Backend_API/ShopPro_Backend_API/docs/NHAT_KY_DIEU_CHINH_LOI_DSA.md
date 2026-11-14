# HƯỚNG DẪN & NHẬT KÝ ĐIỀU CHỈNH BACKEND SHOPPRO
Cập nhật sau code review ngày 13/11/2026. Source gốc trong ZIP được giữ nguyên; đây là bản đã điều chỉnh.

## 1. Build, test và chạy API
```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure              # 6 bo test
./build/Integration/shoppro_cli                         # CLI danh sach lien ket doi
```
REST API: tao Admin bang `./build/Integration/shoppro_api --create-admin admin@shoppro.vn "ShopPro Admin"` roi chay `./build/Integration/shoppro_api` tu thu muc `Backend/`. Huong dan API chi tiet nam trong `README.md`.

## 2. Cấu trúc sau điều chỉnh (khớp sơ đồ kế hoạch)
```
Backend/
  Common/  ProductManager/  ProductSearch/  CartProcessing/   (module của TV1-TV4)
  Integration/   CartController, REST API, HTTP server va CMake
  data/          seed.json; shoppro.json duoc tao khi chay API
  tests/         5 file test + demo_kiem_tra_lien_ket.cpp          (gom ve 1 cho)
```

## 3. Đã sửa gì, vì sao, test nào chứng minh

| Lỗi | Trước | Sau | Test |
|---|---|---|---|
| #1 `xoaNode`/`chenSau` | Truyền node lạ → xóa nhầm, mất node thật, hoặc crash | Có hàm `thuocDanhSach`; node lạ → trả `false`, danh sách không đổi | `test_doubly_linked_list` (nhóm "node la") |
| #2 `taoNode` | Nhận NaN, vô cực, id ≤ 0, tên toàn khoảng trắng | Kiểm tra **cùng bộ quy tắc** với `ProductValidation` | `taoNode: ...` (8 test) |
| #3 tổng số lượng | `int` tràn khi giỏ lớn (UB) | `tinhTongSoLuong` trả `long long`; mỗi dòng ≤ `SO_LUONG_TOI_DA` | TC_CALC_12, 13 |
| #4 hiển thị tiền | NaN/vô cực in ra số âm khổng lồ | In `---`; giá ≤ `GIA_TOI_DA` nên tổng tiền không thành vô cực | TC_FMT_06..09 |
| #5 copy danh sách | `DanhSach b = a` rồi giải phóng 2 lần → crash | Cấm copy (lỗi lúc biên dịch) + destructor tự giải phóng | `tuHuy`, "DanhSach moi tao" |
| #8, #9 code trùng | `tinhTongTien`/`hienThiGioHang` viết 2 nơi | `ProductManager` gọi lại `CartCalculator` / `ProductDisplay` | `test_product_manager` |
| #10 `new` | Kiểm tra `nullptr` sau `new` là code chết | `new (std::nothrow)` → nhánh `LOI_BO_NHO` có thật | — |
| #11 | `using namespace std;` trong header | Đã bỏ, dùng `std::` | biên dịch sạch |
| #13-15 tài liệu | Đường dẫn cũ, test case ghi lệch code | README / TestCases / TestReport / launch.json đã chỉnh | — |
| (thêm) `timNodeTheoID` | Danh sách vòng + ID không có → treo | Giới hạn `soNode` bước như các hàm khác | — |

**Giới hạn dữ liệu mới** (`Common/Node.h`): `GIA_TOI_DA` = 1.000 tỷ đồng, `SO_LUONG_TOI_DA` = 1.000.000 mỗi dòng. Muốn đổi thì sửa 1 chỗ này.

## 4. Giải thích nhanh các kỹ thuật mới (để bạn trình bày được)
- **Destructor (`~DanhSach`)**: hàm tự chạy khi biến bị hủy (ra khỏi `{ }`), nên không còn quên `giaiPhongDanhSach`. Ý tưởng này tên là RAII.
- **`= delete`**: báo cho trình biên dịch "cấm làm điều này". Copy một `DanhSach` chỉ chép 2 con trỏ, hai bản cùng trỏ vào một dãy node, rồi cùng `delete` → lỗi double free. Cấm copy là cách chặn từ gốc.
- **`std::isfinite(x)`**: đúng khi `x` không phải NaN và không phải vô cực. Trước đây `NaN < 0` là `false` nên NaN lọt qua phép kiểm `gia < 0`.
- **`new (std::nothrow)`**: hết bộ nhớ thì trả `nullptr` thay vì ném ngoại lệ, để `if (nodeMoi == nullptr)` thật sự có nghĩa.
- **`long long`**: cộng nhiều số `int` có thể vượt ~2,1 tỷ; `long long` chứa tới ~9,2 tỉ tỉ.

## 5. Ảnh hưởng tới từng thành viên khi gộp code
- **Mọi người:** test chuyển về `tests/`; include dùng `-ICommon -IProductManager ...` (CMake đã lo).
- **TV2:** `tinhTongTien`, `hienThiGioHang` giờ gọi sang TV4/TV3 → khi build riêng phải kèm `CartCalculator.cpp`, `ProductDisplay.cpp`.
- **TV4:** `tinhTongSoLuong` trả `long long` (code cũ `int x = ...` vẫn chạy nhưng nên đổi sang `long long`).
- **TV5 / Frontend:** gọi qua `CartController` (một cửa); số lượng ≤ 1.000.000, giá ≤ 1.000 tỷ.

## 6. Giới hạn hiện tại
1. **Tên tiếng Việt có dấu** (lỗi #6): sắp xếp/tìm theo tên đang đúng với tên không dấu, sai với có dấu. Hai hướng: lưu thêm tên không dấu để tìm/sắp xếp, hoặc dùng thư viện Unicode. Nên chốt cùng thiết kế MySQL.
2. **Tiền dùng `double`** (lỗi #7): VND không có số lẻ nên nên đổi sang `long long` (đồng). Đổi kiểu `gia` kéo theo sửa toàn bộ test, nên cần cả nhóm đồng ý một lần.
3. API REST mới dùng JSON làm kho dữ liệu chung, chưa chuyển sang database. API dùng PBKDF2 để lưu mật khẩu dạng hash, access token tồn tại trong bộ nhớ và hết hạn sau 8 giờ; khởi động lại Backend sẽ yêu cầu đăng nhập lại.
4. Product search trong REST hỗ trợ chuyển chữ thường UTF-8 cho tiếng Việt; phần tìm kiếm/sắp xếp DSA cũ vẫn giữ giới hạn như mô tả ở trên.
