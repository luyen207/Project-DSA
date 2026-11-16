# Test Report - Cart Processing Module

**Module:** CartCalculator + ProductSort  
**Người thực hiện:** Vũ Nguyễn Nhã Uyên (TV4)  
**Ngày kiểm thử:** 13/11/2026  
**Phiên bản:** 1.0  

---

## 1. Tổng quan

Module Cart Processing bao gồm 2 phần chính:
- **CartCalculator**: Tính toán các chỉ tiêu giỏ hàng (tổng tiền, tổng số lượng, giá trung bình)
- **ProductSort**: Sắp xếp sản phẩm theo nhiều tiêu chí (giá, tên, ID)

Mục tiêu kiểm thử:
- Xác minh tính đúng đắn của các hàm tính toán
- Kiểm tra độ chính xác của thuật toán sắp xếp Bubble Sort
- Đảm bảo xử lý an toàn các trường hợp biên
- Kiểm tra tích hợp với module Common (TV1)

---

## 2. Môi trường kiểm thử

| Thông tin | Chi tiết |
|-----------|----------|
| Hệ điều hành | Windows 11 |
| Trình biên dịch | G++ (MinGW) |
| Chuẩn C++ | C++17 |
| Sanitizer | AddressSanitizer + UndefinedBehaviorSanitizer |
| Ngày thực hiện | 13/11/2026 |

**Lệnh biên dịch:**
```bash
g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -g \
    -I"../Cấu trúc DSLK đôi Update" \
    CartCalculator.cpp ProductSort.cpp \
    "../Cấu trúc DSLK đôi Update/Node.cpp" \
    "../Cấu trúc DSLK đôi Update/DoublyLinkedList.cpp" \
    test_cart_calculator.cpp -o test_cart
```

---

## 3. Kết quả kiểm thử

### 3.1. CartCalculator

| Test Case | Tên | Kết quả | Ghi chú |
|-----------|-----|---------|---------|
| TC_CALC_01 | Tính tổng tiền giỏ rỗng | ✅ PASS | Trả về 0 đúng |
| TC_CALC_02 | Tính tổng tiền 1 sản phẩm | ✅ PASS | 100k * 2 = 200k |
| TC_CALC_03 | Tính tổng tiền nhiều sản phẩm | ✅ PASS | Tổng = 4.590.000 |
| TC_CALC_04 | Tính tổng với giá = 0 | ✅ PASS | Xử lý đúng |
| TC_CALC_05 | Tính tổng với giá lớn | ✅ PASS | Không bị tràn double |
| TC_CALC_06 | Tổng số lượng giỏ rỗng | ✅ PASS | Trả về 0 |
| TC_CALC_07 | Tổng số lượng nhiều sp | ✅ PASS | 2+1+1 = 4 |
| TC_CALC_08 | Đếm số loại sản phẩm | ✅ PASS | 3 node = 3 loại |
| TC_CALC_09 | Đếm số loại giỏ rỗng | ✅ PASS | Trả về 0 |
| TC_CALC_10 | Tính giá trung bình | ✅ PASS | 4590k / 4 = 1147.5k |
| TC_CALC_11 | Giá TB giỏ rỗng | ✅ PASS | Xử lý chia 0 đúng |

**Tổng kết CartCalculator:** 11/11 test pass ✅

### 3.2. ProductSort

| Test Case | Tên | Kết quả | Ghi chú |
|-----------|-----|---------|---------|
| TC_SORT_01 | Sắp xếp giỏ rỗng | ✅ PASS | Không crash |
| TC_SORT_02 | Sắp xếp 1 node | ✅ PASS | Không thay đổi |
| TC_SORT_03 | Sắp xếp giá tăng dần | ✅ PASS | 100k -> 200k -> 300k |
| TC_SORT_04 | Đã sắp xếp sẵn | ✅ PASS | Best case |
| TC_SORT_05 | Sắp xếp giá giảm dần | ✅ PASS | 300k -> 200k -> 100k |
| TC_SORT_06 | Sắp xếp theo tên A-Z | ✅ PASS | "Ao" đầu tiên |
| TC_SORT_07 | Không phân biệt hoa/thường | ✅ PASS | "ban phim" < "CHUOT" |
| TC_SORT_08 | Sắp xếp theo ID | ✅ PASS | ID 1 -> 3 -> 5 |
| TC_SORT_09 | Liên kết sau sort | ✅ PASS | prev/next hợp lệ |
| TC_SORT_10 | Giá trùng nhau | ✅ PASS | Stable sort |
| TC_SORT_11 | Danh sách 10 node | ✅ PASS | Xử lý nhiều phần tử |

**Tổng kết ProductSort:** 11/11 test pass ✅

### 3.3. Test tích hợp

| Test Case | Tên | Kết quả | Ghi chú |
|-----------|-----|---------|---------|
| TC_INT_01 | Tổng tiền sau sort | ✅ PASS | Không mất dữ liệu |
| TC_INT_02 | Số loại sau sort | ✅ PASS | Không thêm/xóa node |
| TC_INT_03 | Liên kết vòng | ✅ PASS | Giới hạn soNode, không treo |

**Tổng kết Integration:** 3/3 test pass ✅

---

## 4. Tổng kết chung

| Chỉ tiêu | Kết quả |
|----------|---------|
| **Tổng số test case** | 25 |
| **Số test pass** | 25 ✅ |
| **Số test fail** | 0 |
| **Tỷ lệ thành công** | 100% |
| **Memory leak** | Không phát hiện (kiểm tra bằng AddressSanitizer) |
| **Undefined behavior** | Không phát hiện (kiểm tra bằng UBSan) |
| **Thời gian chạy test** | < 1 giây |

---

## 5. Phân tích chi tiết

### 5.1. Điểm mạnh

✅ **CartCalculator:**
- Tính toán chính xác 100% test case
- Xử lý an toàn giỏ rỗng, không crash
- Xử lý đúng số lớn (double), không tràn
- Xử lý đúng trường hợp chia cho 0 (giá trung bình)
- Có giới hạn vòng lặp (soNode) phòng liên kết vòng

✅ **ProductSort:**
- Sắp xếp chính xác theo 4 tiêu chí: giá tăng, giá giảm, tên, ID
- Bubble Sort đơn giản, dễ hiểu, phù hợp với DSLK đôi
- Không phá vỡ liên kết prev/next (dùng hoanDoiDuLieu)
- Xử lý đúng trường hợp biên (rỗng, 1 node, đã sắp)
- Sắp xếp tên không phân biệt hoa/thường (chuẩn hóa)

✅ **Tích hợp:**
- Tương thích 100% với module Common (TV1)
- Không làm thay đổi dữ liệu khi sắp xếp (tổng tiền không đổi)
- Không thêm/xóa node khi sắp xếp (số loại không đổi)

### 5.2. Trường hợp biên đã test

- ✅ Giỏ hàng rỗng
- ✅ Chỉ có 1 sản phẩm
- ✅ Nhiều sản phẩm (3, 10 node)
- ✅ Giá = 0
- ✅ Giá rất lớn (9.999.999.999)
- ✅ Giá trùng nhau
- ✅ Tên trùng nhau
- ✅ Danh sách đã sắp xếp sẵn
- ✅ Liên kết vòng (lỗi cấu trúc)
- ✅ Không phân biệt hoa/thường khi sắp xếp tên

### 5.3. Vấn đề phát hiện

Không phát hiện lỗi nào. Module hoạt động ổn định 100%.

---

## 6. Độ phủ code

| Module | Hàm | Độ phủ | Ghi chú |
|--------|-----|--------|---------|
| CartCalculator | tinhTongTien | 100% | Đã test đầy đủ |
| CartCalculator | tinhTongSoLuong | 100% | Đã test đầy đủ |
| CartCalculator | demSoLoaiSanPham | 100% | Đã test đầy đủ |
| CartCalculator | tinhGiaTrungBinh | 100% | Đã test đầy đủ |
| ProductSort | sapXepTheoGiaTangDan | 100% | Đã test đầy đủ |
| ProductSort | sapXepTheoGiaGiamDan | 100% | Đã test đầy đủ |
| ProductSort | sapXepTheoTen | 100% | Đã test đầy đủ |
| ProductSort | sapXepTheoID | 100% | Đã test đầy đủ |
| ProductSort | chuanHoaTenDeSort | 100% | Gọi gián tiếp |

**Tổng độ phủ:** 100% các hàm public đã được kiểm thử.

---

## 7. Hiệu năng

| Thao tác | Độ phức tạp | Thời gian thực tế |
|----------|-------------|-------------------|
| Tính tổng tiền | O(n) | < 1ms (10 node) |
| Tính tổng số lượng | O(n) | < 1ms (10 node) |
| Đếm số loại | O(1) | < 1μs |
| Tính giá TB | O(n) | < 1ms (10 node) |
| Sắp xếp Bubble Sort | O(n²) | < 5ms (10 node) |

**Nhận xét:** Hiệu năng đủ tốt cho giỏ hàng thông thường (< 100 sản phẩm). Bubble Sort O(n²) là chấp nhận được cho mục đích học tập.

---

## 8. Khuyến nghị

### 8.1. Đã hoàn thành ✅

- ✅ Tất cả chức năng hoạt động chính xác
- ✅ Xử lý đầy đủ trường hợp biên
- ✅ Không có memory leak
- ✅ Không có undefined behavior
- ✅ Tích hợp tốt với module TV1

### 8.2. Cải tiến tương lai (không bắt buộc)

💡 **Nếu cần mở rộng sau này:**
- Thêm sắp xếp theo số lượng
- Thêm sắp xếp theo thành tiền (giá * số lượng)
- Nâng cấp từ Bubble Sort lên Quick Sort hoặc Merge Sort nếu cần xử lý > 1000 sản phẩm
- Thêm chức năng tìm sản phẩm đắt nhất / rẻ nhất

Tuy nhiên, **với phạm vi đồ án hiện tại, module đã hoàn thiện và đạt 100% yêu cầu.**

---

## 9. Kết luận

✅ **Module Cart Processing (TV4) ĐẠT YÊU CẦU**

- Tất cả 25 test case đều PASS (100%)
- Không có lỗi, không có memory leak
- Code sạch, dễ đọc, có comment đầy đủ
- Tích hợp tốt với module Common (TV1)
- Sẵn sàng bàn giao cho TV5 (Nguyễn Quang Nhân) tích hợp giao diện

**Người kiểm thử:** Vũ Nguyễn Nhã Uyên (TV4)  
**Ngày:** 13/11/2026  
**Trạng thái:** ✅ HOÀN THÀNH
