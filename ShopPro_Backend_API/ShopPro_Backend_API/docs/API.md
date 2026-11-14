# ShopPro – Tài liệu REST API

Mọi đường dẫn bắt đầu bằng `/api`. Dữ liệu gửi và nhận là JSON (`Content-Type: application/json`).
Thời gian dùng ISO 8601 UTC (`2026-11-14T09:00:00.000Z`); tiền tệ là số nguyên đồng (VND).

## Quy ước chung

| Nội dung | Quy ước |
|---|---|
| Xác thực | Gửi `Authorization: Bearer <token>`. Token lấy từ API đăng nhập, hết hạn sau **8 giờ**, bị thu hồi khi đăng xuất hoặc khi tài khoản bị khóa. |
| Phân quyền | `Public` (không cần token), `Customer` (token khách), `Admin` (token quản trị). Token khách **không** dùng được cho API Admin và ngược lại. |
| Lỗi | `{"error": "Thông báo tiếng Việt"}` kèm mã HTTP: `400` dữ liệu sai · `401` chưa/hết phiên · `403` sai quyền hoặc tài khoản bị khóa · `404` không thấy · `409` xung đột nghiệp vụ (hết hàng, sai quy trình trạng thái…) · `413/431` yêu cầu quá lớn · `429` quá nhiều lần thử (có header `Retry-After`). |
| Giới hạn | Giá ≤ 10.000.000.000 ₫/sản phẩm · tồn kho ≤ 100.000 · mật khẩu 8–128 ký tự (có cả chữ và số khi đăng ký). |
| Phân trang | Chỉ `GET /api/products` và `GET /api/admin/activity-logs` phân trang: tham số `page` (từ 1), `limit`, kết quả `{items, page, limit, total}`. Các danh sách khác (đơn hàng, khách hàng, sản phẩm/danh mục của Admin) trả về **mảng JSON** thông thường. |

## Công khai

| Phương thức | Đường dẫn | Mô tả |
|---|---|---|
| GET | `/api/health` | Kiểm tra máy chủ sống (số sản phẩm, đơn hàng). Dùng làm health check khi triển khai. |
| GET | `/api/store` | Thông tin cửa hàng (tên, hotline, email, tiền tệ). |
| GET | `/api/products` | Danh sách sản phẩm đang bán. Tham số: `search`, `category` (mã hoặc tên), `minPrice`, `maxPrice`, `inStock=1`, `featured=1`, `bestSelling=1`, `sort` = `price-asc` · `price-desc` · `newest` · `name` · `popular`, `page`, `limit` (≤ 100, mặc định 12). Tìm kiếm không phân biệt dấu. |
| GET | `/api/products/:id` | Chi tiết một sản phẩm (404 nếu đang ẩn). |
| GET | `/api/products/:id/related` | Tối đa 4 sản phẩm cùng danh mục. |
| GET | `/api/categories` | Danh mục kèm `productCount`. |

## Tài khoản khách

| Phương thức | Đường dẫn | Quyền | Mô tả |
|---|---|---|---|
| POST | `/api/auth/register` | Public | `{name, email, phone, password}` → `201 {user, token, expiresAt}`. Giới hạn tần suất theo IP. |
| POST | `/api/auth/login` | Public | `{email, password}` → `{user, token, expiresAt}`. Sai nhiều lần → `429`. Tài khoản bị khóa → `403`. |
| POST | `/api/auth/logout` | Customer | Thu hồi token hiện tại. |
| GET | `/api/auth/me` | Customer | Thông tin tài khoản đang đăng nhập. |
| PATCH | `/api/users/me` | Customer | Cập nhật `name`, `phone`, `address` (gửi trường nào sửa trường đó). |

## Giỏ hàng (danh sách liên kết đôi)

Mọi API sửa giỏ đều trả về **giỏ hàng đầy đủ**:

```json
{
  "userId": "usr-…",
  "items": [ { "product": { "id": 105, "name": "…", "price": 349000, "stock": 96, "image": "/img/products/105.svg", "…": "các trường còn lại của sản phẩm" },
               "quantity": 2, "lineTotal": 698000, "exceedsStock": false } ],
  "total": 698000, "totalQuantity": 2, "distinctCount": 1,
  "dsa": { "size": 1, "head": 105, "tail": 105, "linksValid": true }
}
```

Thứ tự `items` chính là thứ tự các node trong danh sách liên kết, từ head đến tail.

| Phương thức | Đường dẫn | Mô tả |
|---|---|---|
| GET | `/api/cart` | Xem giỏ (tự loại sản phẩm đã xóa/ẩn, cập nhật giá mới nhất). |
| DELETE | `/api/cart` | Xóa toàn bộ giỏ. |
| POST | `/api/cart/items` | `{productId, quantity}` – thêm sản phẩm; đã có thì **cộng dồn**. `409` nếu vượt tồn kho. |
| PATCH | `/api/cart/items/:id` | `{quantity}` – đặt lại số lượng (1…tồn kho). |
| DELETE | `/api/cart/items/:id` | Bỏ một sản phẩm khỏi giỏ. |
| POST | `/api/cart/sort` | `{by}` với `by` = `price-asc` · `price-desc` · `name` · `id`. Sắp xếp lại các node. |
| GET | `/api/cart/search?q=` | Tìm trong giỏ theo tên (không dấu cũng được) → `{query, items, total}`. |
| GET | `/api/cart/structure?direction=forward\|backward` | Cấu trúc danh sách: `{direction, size, head, tail, valid, nodes:[{id,name,quantity,price,prev,next}], textView}`. `valid` là kết quả `kiemTraLienKet()`. |

## Đơn hàng của khách

| Phương thức | Đường dẫn | Mô tả |
|---|---|---|
| POST | `/api/orders` | Đặt hàng từ giỏ: `{customerName?, phone, shippingAddress, note?, paymentMethod: "cod"\|"bank", items?}`. `items` (tùy chọn) là bản đối chiếu giỏ hàng trên màn hình; lệch với giỏ thật → `409`. Trừ tồn kho, xóa giỏ, `201` trả đơn. |
| GET | `/api/orders` | Đơn của tôi, mới nhất trước (mảng JSON). |
| GET | `/api/orders/:id` | Chi tiết một đơn của tôi. |
| POST | `/api/orders/:id/cancel` | Hủy đơn đang `pending` (trả hàng về kho). Trạng thái khác → `409`. |

Yêu cầu: tên người nhận ≥ 2 ký tự (mặc định lấy tên tài khoản), số điện thoại hợp lệ, địa chỉ ≥ 8 ký tự, `paymentMethod` mặc định `cod`.
Đơn trả về có `id, customerId, customerName, email, phone, address, note, paymentMethod, status, total, items[{productId, productName, unitPrice, quantity, image}], createdAt, updatedAt`
(lưu ý: gửi lên là `shippingAddress`, nhận về là `address`).

Mã đơn dạng `SP-YYMMDD-XXXX` (ví dụ `SP-261112-BEF7`). Trạng thái: `pending` → `confirmed` → `shipping` → `completed`; `pending`/`confirmed` có thể sang `cancelled`.

## Trợ lý mua sắm

| Phương thức | Đường dẫn | Quyền | Mô tả |
|---|---|---|---|
| POST | `/api/ai/chat` | Customer | `{message}` → `{answer, products[], comparison, filters:{maxPrice, category}}`. Hiểu ngân sách ("dưới 2 triệu", "500k", "1,5tr"), tên danh mục, "rẻ nhất", "bán chạy", "so sánh". Luật trên dữ liệu cửa hàng, không gọi dịch vụ ngoài. |

## Quản trị (`Admin`)

Đăng nhập: `POST /api/admin/auth/login` (`{email, password}`), đăng xuất `POST /api/admin/auth/logout`, thông tin `GET /api/admin/auth/me`.

| Phương thức | Đường dẫn | Mô tả |
|---|---|---|
| GET | `/api/admin/dashboard` | Số liệu tổng quan: `revenue, todayRevenue, orderCount, todayOrders, pendingOrders, customerCount, productCount, lowStockCount, outOfStockCount, lowStockThreshold`, `revenueSeries` (14 ngày), `statusSeries`, `recentOrders`, `bestSellers`, `lowStock`. |
| GET | `/api/admin/reports/summary?from=YYYY-MM-DD&to=YYYY-MM-DD` | Báo cáo theo khoảng ngày: `revenue, orderCount, completedCount, averageOrderValue, customerCount, productCount, revenueSeries, statusSeries, categorySeries, topProducts, topCustomers`. Doanh thu chỉ tính đơn `completed`. |
| GET · POST | `/api/admin/products` | Danh sách (kể cả đang ẩn) · tạo sản phẩm. |
| GET · PUT · DELETE | `/api/admin/products/:id` | Xem · thay toàn bộ · xóa (gỡ khỏi giỏ của khách). |
| PATCH | `/api/admin/products/:id/stock` | `{stock}` |
| PATCH | `/api/admin/products/:id/visibility` | `{visible: true\|false}` |
| GET · POST | `/api/admin/categories` | Danh sách · tạo. |
| PUT · DELETE | `/api/admin/categories/:id` | Sửa · xóa (`409` nếu còn sản phẩm). |
| GET | `/api/admin/orders` | Tất cả đơn; lọc `status`, `q`. |
| GET | `/api/admin/orders/:id` | Chi tiết đơn. |
| PATCH | `/api/admin/orders/:id/status` | `{status}` – chỉ chuyển theo quy trình hợp lệ; hủy thì hoàn kho. Sai quy trình → `409`. |
| GET | `/api/admin/customers` | Khách hàng kèm số đơn, tổng chi tiêu. |
| GET | `/api/admin/customers/:id/orders` | Đơn của một khách. |
| PATCH | `/api/admin/customers/:id/status` | `{status: "active"\|"locked"}` – khóa sẽ thu hồi mọi phiên của khách. |
| GET · PUT | `/api/admin/settings` | Đọc · lưu cài đặt: `storeName, supportEmail, supportPhone, currency, lowStockThreshold`. |
| GET | `/api/admin/activity-logs` | Nhật ký, mới nhất trước. Lọc `action`, `actorId`, `result` (`success`\|`failure`\|`blocked`), `from`, `to`, `page`, `limit` (≤ 200). |

### Khóa hành động trong nhật ký

`auth.login.succeeded|failed|blocked`, `auth.logout`, `auth.registered`, `auth.register.blocked`, `customer.profile.updated`,
`cart.item_added|item_updated|item_removed|cleared|sorted`, `order.placed`, `order.cancelled`, `admin.order.status_changed`,
`admin.product.created|updated|deleted|stock_updated|visibility_updated`, `admin.category.created|updated|deleted`,
`admin.customer.status_changed`, `admin.settings.updated`.

Mỗi bản ghi: `{id, createdAt, action, actorId, actorRole, targetType, targetId, result, details?, ip?}`.

## Thử nhanh bằng curl

```bash
# Đăng nhập khách rồi xem giỏ
TOKEN=$(curl -s localhost:8080/api/auth/login -H 'Content-Type: application/json' \
        -d '{"email":"khach@shoppro.vn","password":"Demo@12345"}' | python3 -c 'import sys,json;print(json.load(sys.stdin)["token"])')
curl -s localhost:8080/api/cart -H "Authorization: Bearer $TOKEN"
curl -s -X POST localhost:8080/api/cart/sort -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' -d '{"by":"price-desc"}'
curl -s "localhost:8080/api/cart/structure?direction=backward" -H "Authorization: Bearer $TOKEN"
```
