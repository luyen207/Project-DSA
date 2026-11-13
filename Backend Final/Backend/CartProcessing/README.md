# CartProcessing (TV4 - Vu Nguyen Nha Uyen)

Tinh toan va sap xep gio hang (danh sach lien ket doi cua TV1).
Chi **doc** va **sap xep** danh sach, khong them/xoa node.

## File
| File | Noi dung |
|------|----------|
| `CartCalculator.h/.cpp` | `tinhTongTien`, `tinhTongSoLuong`, `demSoLoaiSanPham`, `tinhGiaTrungBinh` |
| `ProductSort.h/.cpp` | `sapXepTheoGiaTangDan`, `sapXepTheoGiaGiamDan`, `sapXepTheoTen`, `sapXepTheoID` |
| `../tests/test_cart_calculator.cpp` | 27 test case |
| `TestCases.md` | Danh sach test case chi tiet |
| `TestReport.md` | Bao cao ket qua kiem thu |

## Phu thuoc (TV1 - Common/)
`DoublyLinkedList.h` (kem `Node.h`). Dung: `struct Node {id, ten, gia, soLuong, prev, next}`,
`struct DanhSach {head, tail, soNode}`, `laRong`, `hoanDoiDuLieu`.

## API

```cpp
// Tinh toan
double tongTien    = CartCalculator::tinhTongTien(ds);
long long tongSoLuong = CartCalculator::tinhTongSoLuong(ds);   // long long: khong tran
int    soLoai      = CartCalculator::demSoLoaiSanPham(ds);
double giaTB       = CartCalculator::tinhGiaTrungBinh(ds);

// Sap xep (thay doi thu tu node trong danh sach)
ProductSort::sapXepTheoGiaTangDan(ds);   // gia thap -> cao
ProductSort::sapXepTheoGiaGiamDan(ds);   // gia cao -> thap
ProductSort::sapXepTheoTen(ds);          // A -> Z (khong phan biet hoa/thuong)
ProductSort::sapXepTheoID(ds);           // ID nho -> lon
```

## Quy tac

- **Tinh toan**: chi doc, khong sua doi danh sach.
- **Sap xep**: doi cho **du lieu** node (id, ten, gia, soLuong), **KHONG** doi con tro prev/next.
- Dung ham `hoanDoiDuLieu(Node* a, Node* b)` cua TV1.
- Sap xep dung thuat toan **Bubble Sort** (don gian, de hieu, phu hop DSLK doi).
- Sap xep theo ten: khong phan biet hoa/thuong (doi chu A-Z thanh a-z truoc khi so sanh).
- Gio rong hoac chi co 1 node: khong lam gi.
- Duyet co gioi han `soNode` buoc nen lien ket hong khong lam treo chuong trinh.

## Thuat toan Bubble Sort

Bubble Sort la thuat toan sap xep don gian, phu hop voi DSLK doi:
- Duyet nhieu lan qua danh sach.
- Moi lan so sanh cac cap node lien tiep.
- Neu sai thu tu thi doi cho du lieu 2 node.
- Sau moi lan duyet, phan tu lon nhat "noi" ve cuoi.
- Do phuc tap: O(n²) - cham nhung du dung cho du an hoc tap.

## Test

```bash
# Tu thu muc Backend/
g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -g \
    -ICommon -ICartProcessing \
    CartProcessing/*.cpp Common/Node.cpp Common/DoublyLinkedList.cpp \
    tests/test_cart_calculator.cpp -o test_cart

./test_cart      # Ky vong: Dat 27 / 27 test
```

## Ket qua kiem thu

Xem chi tiet trong [TestReport.md](TestReport.md).

- Tong so test: 27 (25 ban dau + 2 test them sau review)
- So test dat: 27
- Ti le thanh cong: 100%
- Thoi gian thuc hien: 13/11/2026
- Nguoi thuc hien: Vu Nguyen Nha Uyen (TV4)

## Luu y

- Module nay phu thuoc vao TV1 (Common/) da hoan thanh.
- Khong sua doi code cua TV1, TV2, TV3.
- Chi them chuc nang tinh toan va sap xep.
- Sap xep thay doi thu tu node nhung giu nguyen lien ket prev/next hop le.
