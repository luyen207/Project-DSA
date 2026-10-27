# Hệ thống quản lý giỏ hàng online — Version 3.5

> **Trọng tâm:** hoàn thiện sản phẩm theo hướng giao diện hiện đại, điều hướng rõ ràng, trải nghiệm riêng cho khách hàng và quản trị viên, đồng thời tăng độ ổn định và khả năng bàn giao.

## 1. Mục tiêu

V3.5 là giai đoạn hoàn thiện trên nền nghiệp vụ và dữ liệu của V3. Không nên chỉ thay đổi màu sắc hoặc bố cục; phiên bản này cần cải thiện trải nghiệm, khả năng tìm kiếm/lọc, thông báo, xử lý lỗi, tính nhất quán dữ liệu và chất lượng tổng thể.

## 2. Giao diện quản trị viên

### 2.1. Thanh điều hướng

- Tổng quan.
- Đơn hàng: Danh sách đơn hàng, Tạo đơn hàng, Quản lý trạng thái/luồng đơn.
- Sản phẩm.
- Khách hàng.
- Kho hàng.
- Sản phẩm khuyến mãi.
- Báo cáo.
- Quản trị: nhân viên, vai trò và quyền.
- Cài đặt hệ thống.

Các mục hiển thị tùy theo vai trò thực tế của tài khoản; không phải mọi nhân viên đều có quyền quản trị toàn hệ thống.

### 2.2. Trang Tổng quan (Dashboard)

- Thẻ KPI: tổng đơn hàng, doanh thu, sản phẩm đã bán và khách hàng mới.
- Biểu đồ doanh thu theo tháng hoặc khoảng thời gian được chọn.
- Biểu đồ/tóm tắt trạng thái đơn hàng.
- Danh sách đơn hàng gần đây.
- Sản phẩm bán chạy.
- Cảnh báo kho hàng sắp hết và thông báo nghiệp vụ.
- Bộ lọc thời gian rõ ràng; mọi số liệu phải lấy từ nguồn dữ liệu thống nhất.

### 2.3. Danh sách đơn hàng

Bảng nên có tối thiểu: mã đơn, tên khách hàng, ngày đặt, ngày giao dự kiến, tổng tiền, trạng thái đơn và thao tác xem/sửa theo quyền. Nên bổ sung tìm kiếm, bộ lọc trạng thái, lọc khoảng ngày, phân trang và trang chi tiết đơn.

Ngày giao dự kiến phải được phân biệt với ngày giao thực tế. Thao tác cập nhật trạng thái phải kiểm tra chuyển trạng thái hợp lệ và lưu lịch sử thay đổi.

### 2.4. Các phân hệ quản trị khác

- **Sản phẩm:** danh sách, chi tiết, thêm/sửa, phân loại, tìm kiếm, lọc và ngừng kinh doanh.
- **Khách hàng:** hồ sơ, trạng thái tài khoản, lịch sử mua hàng trong phạm vi quyền.
- **Kho hàng:** tồn hiện tại, nhập/xuất/điều chỉnh, cảnh báo và lịch sử giao dịch.
- **Khuyến mãi:** mã, thời hạn, điều kiện, mức giảm, lượt sử dụng và trạng thái chương trình.
- **Báo cáo:** doanh thu, số đơn, tỷ lệ trạng thái, sản phẩm bán chạy; xuất file nếu nằm trong phạm vi yêu cầu.
- **Quản trị:** tài khoản nhân viên, vai trò, phân quyền và lịch sử thao tác quan trọng.
- **Cài đặt hệ thống:** cấu hình không nhạy cảm, múi giờ/định dạng ngày, thông số hiển thị và tùy chọn được phép.

## 3. Giao diện khách hàng

Khách hàng không được nhìn thấy menu quản trị. Các trang chính gồm:

1. **Cửa hàng:** danh mục, tìm kiếm, lọc, chi tiết sản phẩm, giá và tình trạng còn hàng.
2. **Khuyến mãi:** sản phẩm/chương trình đang áp dụng; phải thể hiện rõ điều kiện và thời hạn.
3. **Giỏ hàng:** thêm, xóa, chọn mặt hàng, cập nhật số lượng, xem tạm tính và áp mã giảm giá.
4. **Thanh toán/Đặt hàng:** thông tin nhận hàng, phương thức thanh toán, tổng tiền cuối cùng và xác nhận.
5. **Đơn hàng của tôi:** xem chi tiết, trạng thái đơn, trạng thái thanh toán và lịch sử theo dõi.
6. **Tài khoản:** đăng ký, đăng nhập, đăng xuất, cập nhật hồ sơ và mật khẩu.

Khách hàng chỉ xem và thao tác trên tài nguyên của mình. Backend phải xác minh quyền sở hữu theo phiên đăng nhập/token, không tin ID khách hàng do giao diện tự gửi.

## 4. Quy chuẩn UI/UX

- Giao diện nhất quán về màu sắc, kiểu chữ, khoảng cách, nút và trạng thái.
- Thiết kế có thể sử dụng được trên kích thước màn hình mục tiêu của dự án.
- Có trạng thái loading, không có dữ liệu, thành công, lỗi và mất kết nối.
- Biểu mẫu có nhãn, kiểm tra dữ liệu và thông báo lỗi gần trường nhập.
- Hộp thoại xác nhận với hành động nguy hiểm như hủy đơn hoặc điều chỉnh tồn.
- Bảng dữ liệu có tìm kiếm, lọc và phân trang khi cần.
- Không chỉ dùng màu để phân biệt trạng thái; cần có nhãn chữ rõ ràng.
- Các thao tác quan trọng phải có phản hồi rõ ràng và không khiến người dùng gửi lặp ngoài ý muốn.

## 5. Quy tắc nghiệp vụ cần giữ nguyên từ V3

- Đơn hàng lưu giá và số lượng tại thời điểm xác nhận.
- Giá, giảm giá, tồn kho và tổng tiền được kiểm tra/tính ở Backend.
- Trạng thái đơn hàng, thanh toán và giao hàng là các khái niệm riêng.
- Không cho phép chuyển trạng thái tùy ý.
- Hủy đơn/hoàn tiền/hoàn tồn kho phải được xử lý nhất quán và không lặp.
- Doanh thu trên Dashboard phải tuân theo định nghĩa đã thống nhất.
- Dữ liệu lịch sử đơn hàng phải còn đọc được dù sản phẩm đã ngừng kinh doanh.

## 6. Tiêu chí chất lượng và bảo mật

- Không hiển thị chức năng quản trị cho khách hàng và vẫn kiểm tra quyền ở Backend.
- Không lưu mật khẩu dạng văn bản thuần.
- Không để lộ thông tin kết nối Database, token hoặc stack trace nhạy cảm cho người dùng.
- Không có lỗi nghiêm trọng khi dữ liệu rỗng, truy vấn không có kết quả hoặc API thất bại.
- Tìm kiếm, lọc và phân trang không làm sai tổng số hoặc dữ liệu hiển thị.
- Kiểm tra các thao tác trên trình duyệt và API; không chỉ đánh giá bằng ảnh chụp giao diện.
- Có thể khởi động môi trường từ hướng dẫn README mà không cần suy đoán cấu hình.

## 7. Kiểm thử nghiệm thu V3.5

- Kiểm tra luồng khách hàng: đăng ký → đăng nhập → tìm sản phẩm → thêm vào giỏ → áp mã → đặt hàng → theo dõi đơn.
- Kiểm tra luồng quản trị: đăng nhập → xem Dashboard → tìm đơn → xem chi tiết → cập nhật trạng thái đúng quy tắc.
- Kiểm tra luồng kho: nhập/điều chỉnh kho → đặt hàng → xác minh số lượng → hủy đơn (nếu được phép) → xác minh số lượng hoàn lại đúng một lần.
- Kiểm tra dữ liệu Dashboard đối chiếu với các đơn hàng mẫu.
- Kiểm tra quyền truy cập bằng tài khoản có vai trò khác nhau.
- Kiểm tra lỗi mạng, API trả lỗi, dữ liệu rỗng, form không hợp lệ và thao tác gửi lặp.
- Kiểm tra responsive trên các kích thước màn hình trong phạm vi hỗ trợ.
- Chạy lại bộ kiểm thử của V1, V2 và V3 để phát hiện hồi quy.

## 8. Danh sách bàn giao

- Mã nguồn Frontend, Backend và các file cấu hình build.
- Schema/migration Database và dữ liệu mẫu không chứa thông tin cá nhân thật.
- `.env.example` cùng hướng dẫn cấu hình; không bàn giao bí mật bằng cách commit vào Git.
- Postman collection hoặc tài liệu API.
- Hướng dẫn cài đặt, chạy, kiểm thử và khắc phục lỗi phổ biến.
- Danh sách chức năng đã hoàn thành/chưa hoàn thành và các giới hạn còn tồn tại.
- Hướng dẫn tài khoản demo an toàn; không dùng mật khẩu thực tế của thành viên.
- Ảnh chụp giao diện chính nếu giảng viên yêu cầu báo cáo.

## 9. Tiêu chí hoàn thành Version 3.5

- Giao diện khách hàng và quản trị viên tách biệt rõ theo vai trò.
- Các luồng nghiệp vụ chính hoạt động xuyên suốt Frontend → Backend → Database.
- Dashboard và bảng dữ liệu phản ánh đúng nguồn dữ liệu.
- Không còn lỗi nghiêm trọng trong các trường hợp kiểm thử đã thống nhất.
- Các lỗi và trạng thái hệ thống được thông báo rõ ràng.
- Có tài liệu bàn giao và hướng dẫn tái lập môi trường.

## 10. Nguyên tắc phát triển

Ưu tiên độ đúng và ổn định của nghiệp vụ trước hiệu ứng giao diện. Mọi thay đổi API hoặc schema phải được cập nhật trong tài liệu, được review qua Pull Request và có kiểm thử hồi quy. V3.5 chỉ được xem là hoàn tất khi phiên bản có thể được người khác cài đặt, chạy và kiểm thử theo hướng dẫn.
