# ProductSearch (TV3 - Nguyen Dinh Nhat Ha My)

Tim kiem va hien thi san pham trong gio hang (danh sach lien ket doi cua TV1).
Chi **doc** danh sach, khong them/sua/xoa node.

## File
| File | Noi dung |
|------|----------|
| `ProductSearch.h/.cpp` | `timTheoId`, `timTheoTen`, `chuanHoaTen` |
| `ProductDisplay.h/.cpp` | `hienThiTatCa` (xuoi/nguoc), `hienThiDanhSach`, `hienThiMotSanPham`, `hienThiKhongTimThayTen`, `dinhDangTien` |
| `tests/test_product_search.cpp` | 22 test case |

## Phu thuoc (TV1 - Common/)
`DoublyLinkedList.h` (kem `Node.h`). Dung: `struct Node {id, ten, gia, soLuong, prev, next}`,
`struct DanhSach {head, tail, soNode}`, `laRong`, `timNodeTheoID`.

## API
```cpp
// Tim kiem
const Node*               ProductSearch::timTheoId(ds, 1);        // nullptr = khong thay
std::vector<const Node*>  ProductSearch::timTheoTen(ds, "chuot"); // rong = khong thay

// Hien thi
ProductDisplay::hienThiTatCa(ds, true);    // xuoi: head -> tail (next)
ProductDisplay::hienThiTatCa(ds, false);   // nguoc: tail -> head (prev)
ProductDisplay::hienThiDanhSach(ketQuaTimTen);
ProductDisplay::hienThiMotSanPham(ketQuaTimId, 1);
```
Moi ham hien thi nhan them `std::ostream&` (mac dinh `std::cout`) de kiem thu.

## Quy tac
- Tim theo ten: **khong phan biet** hoa/thuong, khop **mot phan**; tu khoa rong tra ve rong.
  Chi ha chu A-Z (du lieu nhom dung ten khong dau).
- Con tro tra ve tro thang vao node trong danh sach: **khong giu** qua thao tac them/sua/xoa.
- `gia` la `double`: hien thi lam tron den dong, dinh dang `1.250.000`.
- Duyet co gioi han `soNode` buoc nen lien ket hong khong lam treo chuong trinh.

## Test
```
g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -g \
    -I<thu muc Common> -I<thu muc ProductSearch> \
    ProductSearch/*.cpp Common/Node.cpp Common/DoublyLinkedList.cpp \
    tests/test_product_search.cpp -o chay_test
./chay_test      # ky vong: Dat 22 / 22 test
```
