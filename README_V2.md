# Hệ thống quản lý giỏ hàng online — Version 2

> **Trọng tâm:** bổ sung tài khoản, đăng nhập, phân quyền khách hàng/quản trị viên và nâng cấp trải nghiệm sử dụng trên nền nghiệp vụ giỏ hàng của V1.

## 1. Mục tiêu

Mở rộng V1 để người dùng có vai trò rõ ràng và chỉ thực hiện được các chức năng được cho phép. DSLK đôi và các nghiệp vụ giỏ hàng cốt lõi tiếp tục được duy trì; không viết lại cấu trúc dữ liệu nếu không có lý do kỹ thuật rõ ràng.

## 2. Chức năng khách hàng

- Đăng ký tài khoản, đăng nhập và đăng xuất.
- Xem danh sách sản phẩm mẫu hoặc danh sách sản phẩm hiện có trong hệ thống.
- Thêm sản phẩm vào giỏ hàng.
- Xem, chọn, cập nhật số lượng và xóa sản phẩm trong giỏ.
- Thay thế sản phẩm đã chọn bằng sản phẩm khác theo quy tắc nghiệp vụ.
- Xem tổng tiền dự kiến và tạo đơn hàng cơ bản.
- Chỉ truy cập giỏ hàng và dữ liệu thuộc phạm vi tài khoản của chính mình.

## 3. Chức năng quản trị viên

- Đăng nhập với tài khoản quản trị được cấp.
- Xem danh sách sản phẩm và dữ liệu giỏ hàng/đơn hàng trong phạm vi phiên bản hỗ trợ.
- Thêm, sửa, ẩn/ngừng sử dụng hoặc xóa mềm sản phẩm theo quyền được cấp.
- Quản lý thông tin sản phẩm và xử lý các yêu cầu nghiệp vụ cơ bản.
- Đăng xuất và được bảo vệ khỏi truy cập trái phép vào chức năng quản trị.

> V2 chưa bắt buộc có đầy đủ quản lý kho, chương trình khuyến mãi, báo cáo chuyên sâu hay Database; các phần này thuộc phạm vi V3. Nếu cần lưu tài khoản lâu dài, nhóm phải chốt cơ chế lưu trữ thay vì chỉ lưu trong bộ nhớ.

## 4. Phân quyền tối thiểu

| Chức năng | Khách hàng | Quản trị viên |
|---|---|---|
| Đăng ký tài khoản khách hàng | Có | Không cần |
| Đăng nhập/đăng xuất | Có | Có |
| Quản lý giỏ hàng của mình | Có | Theo yêu cầu nghiệp vụ |
| Xem danh mục sản phẩm | Có | Có |
| Thêm/sửa/ẩn sản phẩm | Không | Có |
| Xem dữ liệu của mọi khách | Không | Chỉ khi được phép |
| Quản lý tài khoản/quyền | Không | Chỉ vai trò quản trị được ủy quyền |

Phân quyền phải được kiểm tra ở lớp xử lý nghiệp vụ, không chỉ dựa vào việc ẩn lựa chọn trong menu.

## 5. Định hướng giao diện

- Có thể tiếp tục dùng Console được tổ chức tốt hơn, giao diện terminal tương tác hoặc chuyển dần sang giao diện khác theo yêu cầu học phần.
- Tách phần giao diện khỏi phần xử lý tài khoản, giỏ hàng và quyền.
- Mỗi màn hình/menu cần có đường quay lại, thông báo lỗi rõ ràng và xử lý lựa chọn không hợp lệ.
- Nếu dự kiến chuyển sang web ở V3/V3.5, không gắn toàn bộ nghiệp vụ vào `main.cpp`; tạo các lớp/service có thể được gọi từ giao diện khác.

## 6. Thiết kế module gợi ý

```text
project/
├── include/
│   ├── auth/AuthService.h
│   ├── auth/User.h
│   ├── auth/Role.h
│   ├── cart/Cart.h
│   ├── cart/DoublyLinkedList.h
│   ├── product/Product.h
│   └── ui/Menu.h
├── src/
│   ├── auth/AuthService.cpp
│   ├── cart/Cart.cpp
│   ├── product/Product.cpp
│   ├── ui/Menu.cpp
│   └── main.cpp
├── tests/
└── README_V2.md
```

Đây là gợi ý tổ chức, không phải yêu cầu bắt buộc về tên file. Nếu nhóm đã có cấu trúc của V1, hãy mở rộng có kiểm soát và cập nhật tài liệu thay vì tạo nhiều class trùng chức năng.

## 7. Lưu ý bảo mật

- Không lưu mật khẩu dạng văn bản thuần; dùng thuật toán băm mật khẩu phù hợp nếu triển khai đăng nhập thực tế.
- Không dùng tên người dùng hoặc ID do client tự gửi làm bằng chứng về quyền sở hữu dữ liệu.
- Từ chối truy cập nếu chưa đăng nhập hoặc không có quyền.
- Không đưa mật khẩu, token hay thông tin nhạy cảm vào log hoặc repository.
- Dữ liệu quản trị không được hiển thị cho khách hàng.

## 8. Kiểm thử

- Đăng nhập đúng, sai mật khẩu, tài khoản không tồn tại và tài khoản bị khóa.
- Từ chối truy cập chức năng quản trị khi đăng nhập bằng khách hàng.
- Khách hàng A không thể sửa/xóa giỏ hàng của khách hàng B.
- Đăng xuất phải kết thúc phiên theo cách triển khai.
- Thử đầu vào rỗng, quá dài, sai định dạng và lựa chọn menu không hợp lệ.
- Xác minh các thao tác trên giỏ hàng vẫn giữ đúng liên kết DSLK đôi như V1.

## 9. Tiêu chí hoàn thành Version 2

- Đăng nhập/đăng xuất hoạt động theo vai trò.
- Khách hàng và quản trị viên nhìn thấy đúng chức năng của mình.
- Các thao tác bị cấm được từ chối tại lớp nghiệp vụ.
- Các chức năng giỏ hàng của V1 không bị hỏng sau khi bổ sung tài khoản.
- Có tài khoản thử nghiệm, hướng dẫn chạy và kiểm thử quyền.
- Nêu rõ dữ liệu tài khoản được lưu ở đâu và có tồn tại sau khi khởi động lại hay không.

## 10. Những gì chưa thuộc V2

Database nghiệp vụ đầy đủ, quản lý kho nâng cao, mã giảm giá, luồng giao hàng, dashboard phân tích và giao diện quản trị hiện đại được hoàn thiện ở V3/V3.5.
