# Hệ thống quản lý giỏ hàng online — Version 1

> **Phiên bản nền tảng:** C++ Console, tập trung vào cấu trúc dữ liệu **Danh sách liên kết đôi (Doubly Linked List)** và các nghiệp vụ giỏ hàng cơ bản.

## 1. Mục tiêu

Xây dựng phiên bản đầu tiên có thể chạy độc lập trên Console, chứng minh việc sử dụng danh sách liên kết đôi để quản lý các mặt hàng trong giỏ. Phiên bản này ưu tiên tính đúng đắn của thuật toán, quản lý bộ nhớ và nghiệp vụ cơ bản; chưa yêu cầu đăng nhập, giao diện web hay Database.

## 2. Phạm vi chức năng

- Hiển thị danh sách mặt hàng mẫu.
- Thêm mặt hàng vào giỏ hàng.
- Xem toàn bộ giỏ hàng.
- Tìm mặt hàng theo ID.
- Cập nhật số lượng hoặc thông tin được phép sửa.
- Xóa một mặt hàng khỏi giỏ.
- Tính thành tiền từng mặt hàng và tổng tiền giỏ hàng.
- Chọn mặt hàng để tạo đơn hàng cơ bản.
- Hiển thị thông báo rõ ràng khi thao tác thành công hoặc thất bại.

## 3. Yêu cầu cấu trúc dữ liệu bắt buộc

Mỗi node mặt hàng cần có tối thiểu:

- `id`: mã mặt hàng.
- `name`: tên hàng.
- `price`: đơn giá.
- `quantity`: số lượng.
- `prev`: con trỏ đến node trước.
- `next`: con trỏ đến node sau.

Danh sách quản lý tối thiểu bằng `head` và `tail`. Có thể bổ sung `size` để lưu số node hiện có.

### Các quy tắc phải giữ đúng

1. Danh sách rỗng: `head == nullptr` và `tail == nullptr`.
2. Với node đầu: `head->prev == nullptr`.
3. Với node cuối: `tail->next == nullptr`.
4. Khi nối hai node A và B: `A->next == B` đồng thời `B->prev == A`.
5. Khi xóa node, phải cập nhật cả liên kết `prev` và `next`, đồng thời giải phóng bộ nhớ đúng cách.
6. Không chấp nhận ID rỗng/trùng lặp nếu quy ước dự án yêu cầu ID duy nhất.
7. Số lượng phải là số nguyên dương; giá không được âm.

## 4. Quy tắc nghiệp vụ

- Thành tiền mặt hàng = đơn giá × số lượng.
- Tổng tiền giỏ = tổng thành tiền của các mặt hàng được tính vào giỏ/đơn hiện tại.
- Chỉ tạo đơn khi có ít nhất một mặt hàng được chọn.
- Việc chọn mặt hàng để mua không được tự động xóa các mặt hàng chưa chọn.
- Nếu thêm sản phẩm có ID đã tồn tại trong giỏ, nhóm cần thống nhất chính sách. Khuyến nghị: cộng số lượng vào node hiện có sau khi kiểm tra dữ liệu hợp lệ.
- Giá tiền nên được lưu bằng kiểu số nguyên theo đơn vị đồng Việt Nam nếu không cần phần lẻ; tránh dùng `float`/`double` làm chuẩn tiền tệ.

## 5. Cấu trúc mã nguồn gợi ý

```text
project/
├── include/
│   ├── CartNode.h
│   ├── DoublyLinkedList.h
│   ├── Cart.h
│   └── Order.h
├── src/
│   ├── DoublyLinkedList.cpp
│   ├── Cart.cpp
│   ├── Order.cpp
│   └── main.cpp
├── tests/
│   └── test_doubly_linked_list.cpp
└── README_V1.md
```

Đây là cấu trúc gợi ý, nhóm có thể điều chỉnh nhưng nên tách phần giao diện Console khỏi phần xử lý danh sách và nghiệp vụ.

## 6. Giao diện Console dự kiến

```text
========== QUAN LY GIO HANG ==========
1. Xem danh sach san pham
2. Them mat hang vao gio
3. Xem gio hang
4. Cap nhat so luong
5. Xoa mat hang
6. Chon mat hang tao don
0. Thoat
Lua chon:
```

Có thể hiển thị tiếng Việt có dấu nếu môi trường biên dịch/Console hỗ trợ ổn định.

## 7. Cách biên dịch và chạy

Yêu cầu: trình biên dịch C++ hỗ trợ chuẩn được nhóm thống nhất (khuyến nghị C++17) và hệ thống build được ghi rõ trong repository.

Ví dụ biên dịch trực tiếp với `g++` khi các file `.cpp` nằm trong `src/`:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -o cart_app
```

Chạy trên Windows:

```powershell
.\cart_app.exe
```

Chạy trên Linux/macOS:

```bash
./cart_app
```

Nếu dự án dùng CMake, bổ sung `CMakeLists.txt` và hướng dẫn build tương ứng; không trộn hai cách build thiếu thống nhất.

## 8. Kiểm thử bắt buộc

- Thêm node khi danh sách rỗng.
- Thêm node vào đầu/cuối danh sách.
- Xóa node duy nhất.
- Xóa node đầu, node cuối và node ở giữa.
- Cập nhật số lượng và tính tổng tiền.
- Thử ID không tồn tại, ID trùng, số lượng bằng 0/âm và giá âm.
- Xem giỏ rỗng và tạo đơn khi chưa chọn mặt hàng.
- Duyệt danh sách từ đầu đến cuối và từ cuối về đầu để xác minh `prev`/`next`.

## 9. Tiêu chí hoàn thành Version 1

- Chương trình biên dịch và chạy được trên môi trường đã thống nhất.
- Các thao tác thêm, xem, sửa, xóa và tạo đơn cơ bản hoạt động đúng.
- Liên kết hai chiều luôn nhất quán sau mọi thao tác.
- Không rò rỉ bộ nhớ trong các thao tác xóa/giải phóng danh sách (nên kiểm tra destructor và quyền sở hữu node).
- Có dữ liệu mẫu, hướng dẫn chạy và danh sách kiểm thử.
- Thành viên khác có thể clone repository và chạy theo README mà không phải tự đoán lệnh.

## 10. Ngoài phạm vi

Đăng nhập, phân quyền, Database, thanh toán thật, tích hợp vận chuyển, Dashboard web và quản lý khuyến mãi nâng cao được để dành cho các phiên bản sau.
