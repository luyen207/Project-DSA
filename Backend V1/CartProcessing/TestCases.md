# Test Cases - Cart Processing Module

**Người thực hiện:** Vũ Nguyễn Nhã Uyên (TV4)  
**Ngày:** 13/11/2026  
**Module:** CartCalculator + ProductSort

---

## 1. Test Cases - CartCalculator

### TC_CALC_01: Tính tổng tiền giỏ rỗng
- **Input:** Danh sách rỗng
- **Expected:** `tinhTongTien(ds) == 0`
- **Mục đích:** Kiểm tra xử lý giỏ hàng rỗng

### TC_CALC_02: Tính tổng tiền 1 sản phẩm
- **Input:** 1 node (id=1, gia=100000, soLuong=2)
- **Expected:** `tinhTongTien(ds) == 200000`
- **Mục đích:** Tính toán cơ bản 1 sản phẩm

### TC_CALC_03: Tính tổng tiền nhiều sản phẩm
- **Input:** 3 node với gia, soLuong khác nhau
- **Expected:** Tổng = sum(gia * soLuong) của tất cả node
- **Mục đích:** Tính tổng nhiều sản phẩm

### TC_CALC_04: Tính tổng tiền với giá = 0
- **Input:** 1 node có gia = 0
- **Expected:** `tinhTongTien(ds) == 0`
- **Mục đích:** Xử lý trường hợp giá = 0

### TC_CALC_05: Tính tổng tiền với giá lớn
- **Input:** gia = 9999999999.0, số lượng 1
- **Expected:** `tinhTongTien(ds) == 9999999999.0`
- **Mục đích:** Kiểm tra xử lý số lớn (double)

### TC_CALC_06: Tính tổng số lượng giỏ rỗng
- **Input:** Danh sách rỗng
- **Expected:** `tinhTongSoLuong(ds) == 0`
- **Mục đích:** Xử lý giỏ rỗng

### TC_CALC_07: Tính tổng số lượng nhiều sản phẩm
- **Input:** 3 node (soLuong = 2, 1, 1) — cùng danh sách với TC_CALC_03
- **Expected:** `tinhTongSoLuong(ds) == 4`
- **Mục đích:** Tính tổng số lượng chính xác

### TC_CALC_08: Đếm số loại sản phẩm
- **Input:** 3 node
- **Expected:** `demSoLoaiSanPham(ds) == 3`
- **Mục đích:** Đếm số node trong danh sách

### TC_CALC_09: Đếm số loại giỏ rỗng
- **Input:** Danh sách rỗng
- **Expected:** `demSoLoaiSanPham(ds) == 0`
- **Mục đích:** Xử lý giỏ rỗng

### TC_CALC_10: Tính giá trung bình
- **Input:** 3 node (250k×2, 890k×1, 3200k×1) — cùng danh sách với TC_CALC_03
- **Expected:** giaTB = 4.590.000 / 4 = 1.147.500 (sai số < 1)
- **Mục đích:** Tính giá trung bình chính xác

### TC_CALC_11: Tính giá trung bình giỏ rỗng
- **Input:** Danh sách rỗng
- **Expected:** `tinhGiaTrungBinh(ds) == 0`
- **Mục đích:** Xử lý chia cho 0

---

## 2. Test Cases - ProductSort

### TC_SORT_01: Sắp xếp giá tăng dần - giỏ rỗng
- **Input:** Danh sách rỗng
- **Expected:** Không làm gì, không crash
- **Mục đích:** Xử lý giỏ rỗng

### TC_SORT_02: Sắp xếp giá tăng dần - 1 node
- **Input:** 1 node duy nhất
- **Expected:** Danh sách không đổi
- **Mục đích:** Không cần sắp xếp 1 phần tử

### TC_SORT_03: Sắp xếp giá tăng dần - nhiều node
- **Input:** 3 node (gia = 300k, 100k, 200k)
- **Expected:** Sau sort: 100k -> 200k -> 300k
- **Mục đích:** Sắp xếp tăng dần chính xác

### TC_SORT_04: Sắp xếp giá tăng dần - đã sắp sẵn
- **Input:** 3 node đã theo thứ tự tăng dần
- **Expected:** Thứ tự không đổi
- **Mục đích:** Best case của bubble sort

### TC_SORT_05: Sắp xếp giá giảm dần
- **Input:** 3 node (gia = 100k, 300k, 200k)
- **Expected:** Sau sort: 300k -> 200k -> 100k
- **Mục đích:** Sắp xếp giảm dần chính xác

### TC_SORT_06: Sắp xếp theo tên A-Z
- **Input:** 3 node (ten = "Chuot", "Ban phim", "Ao")
- **Expected:** Sau sort: "Ao" -> "Ban phim" -> "Chuot"
- **Mục đích:** Sắp xếp theo tên

### TC_SORT_07: Sắp xếp tên không phân biệt hoa/thường
- **Input:** 2 node (ten = "CHUOT", "ban phim")
- **Expected:** "ban phim" trước "CHUOT" (B < C)
- **Mục đích:** Không phân biệt hoa thường khi sort

### TC_SORT_08: Sắp xếp theo ID tăng dần
- **Input:** 3 node (id = 5, 1, 3)
- **Expected:** Sau sort: id 1 -> 3 -> 5
- **Mục đích:** Sắp xếp theo ID

### TC_SORT_09: Kiểm tra liên kết prev/next sau sort
- **Input:** 3 node
- **Expected:** Sau sort, kiemTraLienKet(ds) == true
- **Mục đích:** Liên kết prev/next vẫn hợp lệ

### TC_SORT_10: Sắp xếp với giá trùng nhau
- **Input:** 3 node có cùng giá
- **Expected:** Kết quả vẫn thỏa "giá tăng dần" (test hiện chỉ kiểm tra điều kiện này, chưa kiểm tra tính ổn định)
- **Mục đích:** Xử lý giá trùng

### TC_SORT_11: Sắp xếp danh sách lớn
- **Input:** 10 node giá 1000, 900, ..., 100 (xếp giảm dần)
- **Expected:** Sau sort tăng dần, mỗi node.gia <= node.next->gia
- **Mục đích:** Test với nhiều phần tử

---

## 3. Test Cases - Tích hợp

### TC_INT_01: Tính tổng sau khi sắp xếp
- **Input:** 3 node, sort tăng dần rồi tính tổng tiền
- **Expected:** Tổng tiền không đổi trước và sau sort
- **Mục đích:** Sắp xếp không làm mất dữ liệu

### TC_INT_02: Đếm số loại sau sort
- **Input:** 3 node, sort rồi đếm
- **Expected:** Số loại không đổi
- **Mục đích:** Sort không thêm/xóa node

### TC_INT_03: Liên kết vòng không treo chương trình
- **Input:** Danh sách bị lỗi liên kết vòng (tail->next = head)
- **Expected:** Giới hạn soNode bước, không treo
- **Mục đích:** An toàn khi danh sách bị lỗi

---

### TC_CALC_12: Tổng số lượng vượt INT_MAX (thêm sau review)
- **Input:** 2200 node, mỗi node soLuong = 1.000.000
- **Expected:** `tinhTongSoLuong(ds) == 2.200.000.000` (kiểu `long long`, không tràn)

### TC_CALC_13: Giá trung bình danh sách lớn (thêm sau review)
- **Input:** như TC_CALC_12, giá mỗi node = 1.0
- **Expected:** `tinhGiaTrungBinh(ds) == 1.0`

---

## Tổng kết

- **Tổng số test case:** 27
- **Module CartCalculator:** 13 test cases
- **Module ProductSort:** 11 test cases
- **Test tích hợp:** 3 test cases
- **Độ phủ code:** 100% các hàm public
- **Trường hợp biên:** Giỏ rỗng, 1 node, nhiều node, giá trùng, liên kết lỗi
