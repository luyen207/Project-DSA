# Hệ thống quản lý giỏ hàng online — Version 3

> **Trọng tâm:** tích hợp Database, hoàn thiện nghiệp vụ sản phẩm/kho hàng/đơn hàng/khuyến mãi, xác thực người dùng và báo cáo phân tích.

## 1. Mục tiêu

Chuyển từ chương trình quản lý ở bộ nhớ sang hệ thống có lưu trữ lâu dài, nghiệp vụ có quy tắc rõ và có thể kiểm tra thông qua API hoặc lớp giao tiếp đã thống nhất. C++ tiếp tục là phần xử lý nghiệp vụ nếu đó là yêu cầu của dự án; công nghệ API/HTTP và Frontend cần được nhóm chốt trước khi triển khai.

## 2. Chức năng chính

### 2.1. Tài khoản và phân quyền

- Đăng ký/đăng nhập khách hàng, đăng nhập nhân viên và quản trị viên.
- Phân quyền theo vai trò và kiểm tra quyền ở Backend.
- Quản lý trạng thái tài khoản; bảo vệ dữ liệu cá nhân và dữ liệu mua hàng.

### 2.2. Sản phẩm

- Tạo, xem, cập nhật và ngừng kinh doanh sản phẩm.
- Quản lý mã sản phẩm, tên, mô tả, giá hiện tại, danh mục và trạng thái.
- Tìm kiếm, lọc và phân loại sản phẩm.
- Không xóa cứng dữ liệu sản phẩm đã được tham chiếu bởi đơn hàng lịch sử; ưu tiên lưu trữ mềm.

### 2.3. Giỏ hàng và tạo đơn

- Thêm, chọn, cập nhật số lượng, thay thế và xóa mặt hàng.
- Kiểm tra sản phẩm còn hiệu lực và số lượng hợp lệ.
- Tính lại giá ở Backend trước khi tạo đơn.
- Tạo đơn hàng và chi tiết đơn hàng trong cùng quy trình giao dịch.

### 2.4. Kho hàng

- Theo dõi số lượng tồn.
- Ghi nhận nhập kho, xuất kho và điều chỉnh có lý do.
- Cảnh báo sắp hết/hết hàng.
- Kiểm tra khả năng bán trước khi xác nhận đơn.
- Lưu lịch sử biến động kho để có thể đối soát.

### 2.5. Khuyến mãi

- Tạo mã/chương trình giảm giá với thời hạn.
- Thiết lập điều kiện tối thiểu, mức giảm, mức giảm tối đa và giới hạn lượt sử dụng nếu cần.
- Kiểm tra mã ở Backend; không tin số tiền giảm do Frontend gửi lên.
- Đưa sản phẩm khuyến mãi lên đầu danh sách chỉ là quy tắc sắp xếp hiển thị; vẫn phải có cơ chế lọc/tìm kiếm và quy tắc sắp xếp ổn định.

### 2.6. Trạng thái đơn hàng và thanh toán

- Theo dõi trạng thái đơn: Chờ xác nhận, Đã xác nhận, Đang giao, Đã giao, Đã hủy (có thể mở rộng theo yêu cầu).
- Theo dõi trạng thái thanh toán riêng: Chưa thanh toán, Đang xử lý, Đã thanh toán, Thất bại, Đã hoàn tiền.
- Lưu lịch sử thay đổi trạng thái, thời gian và người thực hiện.
- Quy định rõ khi nào được hủy đơn, hoàn tồn kho hoặc hoàn tiền.

### 2.7. Báo cáo và Dashboard dữ liệu

- Tổng đơn hàng, doanh thu theo quy tắc đã công bố, sản phẩm đã bán và khách hàng mới.
- Biểu đồ doanh thu theo tháng/tuần/ngày.
- Thống kê số đơn theo trạng thái.
- Danh sách sản phẩm bán chạy theo số lượng hoặc doanh thu (phải ghi rõ chỉ số).
- Bộ lọc thời gian và xuất báo cáo nếu nằm trong phạm vi được duyệt.

## 3. Mô hình dữ liệu đề xuất

| Bảng | Mục đích |
|---|---|
| `Users`, `Roles`, `UserRoles` | Tài khoản và quyền |
| `Customers`, `Addresses` (nếu cần) | Hồ sơ khách hàng và địa chỉ |
| `Products`, `Categories` | Sản phẩm và danh mục |
| `Inventory`, `InventoryTransactions` | Tồn kho và lịch sử nhập/xuất |
| `Carts`, `CartItems` | Giỏ hàng và các mặt hàng |
| `Orders`, `OrderItems` | Đơn hàng và chi tiết đơn |
| `Payments` | Giao dịch/trạng thái thanh toán |
| `Promotions`, `PromotionUsages` | Chương trình và lịch sử sử dụng mã |
| `OrderStatusHistory` | Lịch sử trạng thái đơn |

Bảng có thể được điều chỉnh theo phạm vi thực tế. Hãy dùng khóa chính, khóa ngoại, ràng buộc dữ liệu và chỉ mục phù hợp. `OrderItems` cần lưu giá tại thời điểm đặt hàng, không chỉ tham chiếu giá hiện tại ở `Products`.

## 4. Quy tắc dữ liệu và giao dịch

- Dùng transaction khi tạo đơn, ghi chi tiết đơn và cập nhật/giữ tồn kho.
- Ngăn đặt vượt tồn kho khi nhiều yêu cầu diễn ra đồng thời.
- Không tạo hai đơn do client gửi lặp cùng một yêu cầu; cân nhắc cơ chế idempotency cho API tạo đơn.
- Tổng tiền, giảm giá, phí giao hàng và số tiền phải trả được tính/kiểm tra ở Backend.
- Chỉ tính doanh thu theo định nghĩa đã thống nhất, ví dụ đơn đã thanh toán hoặc đơn đã giao thành công; loại trừ hoặc điều chỉnh đơn hủy/hoàn tiền theo chính sách.
- Giá tiền nên dùng kiểu số nguyên theo đồng hoặc kiểu thập phân chính xác phù hợp với Database.

## 5. API tham khảo nếu dùng kiến trúc web

| Method | Endpoint mẫu | Ý nghĩa |
|---|---|---|
| `GET` | `/api/products` | Danh sách sản phẩm |
| `GET` | `/api/products/{id}` | Chi tiết sản phẩm |
| `POST` | `/api/cart/items` | Thêm mặt hàng vào giỏ |
| `PATCH` | `/api/cart/items/{id}` | Cập nhật số lượng |
| `DELETE` | `/api/cart/items/{id}` | Xóa mặt hàng |
| `GET` | `/api/cart` | Xem giỏ hàng |
| `POST` | `/api/orders` | Tạo đơn hàng |
| `GET` | `/api/orders/mine` | Đơn của khách hàng hiện tại |
| `GET` | `/api/admin/orders` | Quản trị xem danh sách đơn |
| `PATCH` | `/api/admin/orders/{id}/status` | Cập nhật trạng thái theo quyền |
| `GET` | `/api/admin/reports/sales` | Dữ liệu báo cáo doanh thu |

Các endpoint trên là thiết kế gợi ý. Trước khi code, nhóm cần chốt schema request/response, mã HTTP, định dạng lỗi, xác thực và phân quyền.

## 6. Cách chạy và cấu hình

Tài liệu triển khai cụ thể phụ thuộc vào stack mà nhóm chọn. Repository cần có các mục sau trước khi nghiệm thu:

1. Phiên bản compiler C++ và lệnh build hoặc `CMakeLists.txt`.
2. Phiên bản Database, file schema/migration và dữ liệu mẫu.
3. File `.env.example` chỉ chứa tên biến và giá trị mẫu không nhạy cảm.
4. Hướng dẫn tạo database/user và cấu hình kết nối.
5. Hướng dẫn khởi động Backend và Frontend theo đúng thứ tự.
6. Bộ Postman collection hoặc file kiểm thử API.

Không commit mật khẩu Database, token hoặc khóa API thật lên GitHub.

## 7. Kiểm thử

- CRUD sản phẩm, tìm kiếm và lọc.
- Kiểm tra tồn kho ở mức 0, 1 và số lượng lớn; hai yêu cầu mua đồng thời.
- Tạo đơn khi giỏ rỗng, mã giảm giá hết hạn hoặc sản phẩm không còn bán.
- Gửi request tạo đơn lặp để kiểm tra đơn trùng.
- Hủy đơn và xác minh tồn kho chỉ được hoàn lại đúng một lần.
- Kiểm tra ranh giới quyền giữa khách hàng, nhân viên và quản trị viên.
- Đối chiếu số liệu Dashboard với dữ liệu đơn hàng mẫu đã biết trước.
- Kiểm tra Database mất kết nối và bảo đảm không tạo dữ liệu dở dang.

## 8. Tiêu chí hoàn thành Version 3

- Dữ liệu nghiệp vụ được lưu và tồn tại sau khi khởi động lại.
- Các nghiệp vụ chính làm việc với Database qua lớp truy cập dữ liệu rõ ràng.
- Đơn hàng/chi tiết đơn/tồn kho được cập nhật nhất quán.
- Phân quyền được thực thi ở Backend.
- Báo cáo có quy tắc tính và số liệu đối chiếu được.
- Có tài liệu cấu hình, dữ liệu mẫu, hướng dẫn chạy và bộ kiểm thử API.
