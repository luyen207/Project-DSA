# Module quản lý sản phẩm trong giỏ hàng (TV2)

Xây dựng trên danh sách liên kết đôi của TV1 (`Node`, `DanhSach`).
Module này lo phần **nghiệp vụ**: thêm, xóa, cập nhật sản phẩm và kiểm tra dữ liệu.

## 1. Cấu trúc file

| File | Nhiệm vụ |
|------|----------|
| `ProductValidation.h/.cpp` | Kiểm tra dữ liệu (id, tên, giá, số lượng), kiểm tra ID trùng, định nghĩa `MaKetQua` |
| `ProductManager.h/.cpp` | Các chức năng: thêm, xóa, cập nhật thông tin, cập nhật số lượng, tổng tiền, hiển thị |
| `../tests/test_product_manager.cpp` | Chương trình kiểm thử (54 test, gồm các trường hợp biên) |

Phụ thuộc: `Node.h/.cpp`, `DoublyLinkedList.h/.cpp` của TV1; `tinhTongTien` và `hienThiGioHang` gọi lại `CartCalculator` (TV4) và `ProductDisplay` (TV3) để không viết trùng.

## 2. Quy tắc dữ liệu hợp lệ

| Trường | Điều kiện |
|--------|-----------|
| `id` | > 0 |
| `ten` | Không rỗng, không chỉ toàn khoảng trắng |
| `gia` | 0 ≤ giá ≤ `GIA_TOI_DA` (1.000 tỷ), là số hữu hạn (không NaN, không vô cực) |
| `soLuong` | 1 ≤ số lượng ≤ `SO_LUONG_TOI_DA` (1.000.000) |

`taoNode` của TV1 kiểm tra **cùng bộ quy tắc**, nên dù tạo node trực tiếp hay qua `themSanPham` kết quả đều giống nhau.

## 3. Danh sách hàm

Mọi hàm thay đổi giỏ hàng đều trả về `MaKetQua`; dùng `moTaKetQua(kq)` để lấy câu thông báo.

| Hàm | Chức năng | Độ phức tạp |
|-----|-----------|-------------|
| `themSanPham(ds, id, ten, gia, soLuong)` | Thêm vào cuối giỏ; nếu ID đã có thì cộng dồn số lượng | O(n) |
| `xoaSanPhamTheoID(ds, id)` | Xóa sản phẩm theo ID | O(n) |
| `capNhatThongTin(ds, id, tenMoi, giaMoi)` | Đổi tên và giá | O(n) |
| `capNhatSoLuong(ds, id, soLuongMoi)` | Đặt lại số lượng (ghi đè) | O(n) |
| `tinhTongTien(ds)` | Tổng giá x số lượng | O(n) |
| `hienThiGioHang(ds)` | In giỏ hàng | O(n) |
| `daTonTaiID(ds, id)` | Kiểm tra ID trùng | O(n) |
| `laIDHopLe / laTenHopLe / laGiaHopLe / laSoLuongHopLe` | Kiểm tra từng trường | O(1) |
| `kiemTraDuLieuSanPham(...)` | Kiểm tra cả 4 trường, trả về lỗi đầu tiên | O(1) |

### Mã kết quả (`MaKetQua`)

`THANH_CONG`, `DA_CONG_DON`, `ID_KHONG_HOP_LE`, `TEN_KHONG_HOP_LE`, `GIA_KHONG_HOP_LE`,
`SO_LUONG_KHONG_HOP_LE`, `ID_KHONG_TON_TAI`, `VUOT_SO_LUONG_TOI_DA`, `LOI_BO_NHO`.

## 4. Cách xử lý các trường hợp đặc biệt

- **Thêm sản phẩm đã tồn tại (trùng ID):** không tạo node mới mà **cộng dồn số lượng** vào node cũ,
  giữ nguyên tên và giá cũ, trả về `DA_CONG_DON`. Muốn đổi tên/giá thì dùng `capNhatThongTin`.
- **Cộng dồn vượt `SO_LUONG_TOI_DA`:** trả về `VUOT_SO_LUONG_TOI_DA`, giỏ hàng không thay đổi.
- **Dữ liệu sai:** trả về mã lỗi tương ứng, giỏ hàng không thay đổi (cả khi ID đã tồn tại).
- **Cập nhật thông tin:** không cho đổi `id` (vì id là khóa); dữ liệu mới sai thì giữ nguyên dữ liệu cũ.
- **Cập nhật số lượng:** ghi đè giá trị cũ; số lượng <= 0 bị từ chối. Muốn bỏ sản phẩm thì dùng `xoaSanPhamTheoID`.
- **Xóa:** xử lý được giỏ rỗng, node đầu, node cuối, node giữa, node duy nhất (nhờ `xoaNode` của TV1).

## 5. Ví dụ sử dụng

```cpp
#include "ProductManager.h"

DanhSach ds;
khoiTaoDanhSach(ds);

themSanPham(ds, 1, "Ao", 100000, 2);          // THANH_CONG
themSanPham(ds, 1, "Ao", 100000, 3);          // DA_CONG_DON -> soLuong = 5
capNhatThongTin(ds, 1, "Ao khoac", 150000);   // THANH_CONG
capNhatSoLuong(ds, 1, 10);                    // THANH_CONG -> soLuong = 10

MaKetQua kq = xoaSanPhamTheoID(ds, 99);       // ID_KHONG_TON_TAI
cout << moTaKetQua(kq) << endl;

hienThiGioHang(ds);
giaiPhongDanhSach(ds);                        // luon goi khi dung xong
```

## 6. Cách build và chạy

```
# Tu thu muc Backend/ (can cmake + g++/clang)
cmake -S Integration -B build -DSHOPPRO_SANITIZE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
(Windows/MinGW: bo `-DSHOPPRO_SANITIZE=ON` vi MinGW thuong khong co AddressSanitizer.)

Build riêng (từ thư mục `Backend/`):

```
g++ -std=c++17 -ICommon -IProductManager -IProductSearch -ICartProcessing \
    tests/test_product_manager.cpp ProductManager/*.cpp ProductSearch/ProductDisplay.cpp \
    CartProcessing/CartCalculator.cpp Common/Node.cpp Common/DoublyLinkedList.cpp -o test_tv2
```

Kết quả đúng: toàn bộ dòng `[PASS]`, dòng cuối là `KET QUA: 54/54 test dat`.

## 7. Các trường hợp biên đã kiểm thử

Giỏ rỗng; id = 0 / âm; tên rỗng / toàn khoảng trắng; giá âm / NaN / vô cực / bằng 0;
số lượng 0 / âm; thêm trùng ID (cùng và khác tên, giá); cộng dồn chạm `SO_LUONG_TOI_DA`;
xóa node đầu / cuối / giữa / duy nhất / không tồn tại / xóa hai lần; cập nhật với dữ liệu sai
(dữ liệu cũ được giữ nguyên); thêm lại sau khi xóa hết. Mọi bước đều kiểm tra `kiemTraLienKet`.

## 8. Quy ước code

Tên biến, tên hàm: tiếng Việt không dấu, camelCase (giống TV1).

## 9. Người thực hiện

Họ tên: Trần Thị Mỹ Dung   Vai trò: Xây dựng chức năng quản lý sản phẩm
