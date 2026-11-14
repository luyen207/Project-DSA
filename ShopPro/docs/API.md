# ShopPro – Tài liệu REST API

Mọi đường dẫn bắt đầu bằng `/api`. Dữ liệu gửi và nhận là JSON (`Content-Type: application/json`).
Thời gian dùng ISO 8601 UTC (`2026-11-14T09:00:00.000Z`); tiền tệ là số nguyên đồng (VND).

## Quy ước chung

| Nội dung | Quy ước |
|---|---|
| Xác thực | Gửi `Authorization: Bearer <token>`. Token lấy từ API đăng nhập, hết hạn sau **8 giờ**, bị thu hồi khi đăng xuất hoặc khi tài khoản bị khóa. |
| Phân quyền | `Public` (không cần token), `Customer` (token khách), `Admin` (token quản trị). Token khách **không** dùng được cho API Admin và ngược lại. |
| Lỗi | `{"error": "Thông báo tiếng Việt"}` kèm mã HTTP: `400` dữ liệu sai · `401` chưa/hết phiên · `403` sai quyền hoặc tài khoản bị khóa · `404` không thấy · `409` xung đột nghiệp vụ (hết hàng, sai quy trình trạng thái…) · `413/431` yêu cầu quá lớn · `429` quá nhiều lần thử (có header `Retry-After`). |
| Giới hạn | Giá ≤ 10.000.000.000 ₫/sản phẩm · tồn kho ≤ 100.000 · mật khẩu 8–128 ký tự (có cả chữ và số khi đăng ký) · mã giảm giá: 1–90 %, mã 3–20 ký tự. |
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
| POST | `/api/orders` | Đặt hàng từ giỏ: `{customerName?, phone, shippingAddress, note?, paymentMethod: "cod"\|"bank", couponCode?, items?}`. `items` (tùy chọn) là bản đối chiếu giỏ hàng trên màn hình; lệch với giỏ thật → `409`. `couponCode` (tùy chọn) là mã giảm giá, được **kiểm tra lại và tính tiền ở máy chủ** (mục *Mã giảm giá* bên dưới). Trừ tồn kho, xóa giỏ, `201` trả đơn. |
| GET | `/api/orders` | Đơn của tôi, mới nhất trước (mảng JSON). |
| GET | `/api/orders/:id` | Chi tiết một đơn của tôi. |
| POST | `/api/orders/:id/cancel` | Hủy đơn đang `pending` (trả hàng về kho). Trạng thái khác → `409`. |

Yêu cầu: tên người nhận ≥ 2 ký tự (mặc định lấy tên tài khoản), số điện thoại hợp lệ, địa chỉ ≥ 8 ký tự, `paymentMethod` mặc định `cod`.
Đơn trả về cho **khách** (`/api/orders…`) có `id, status, items[{product, quantity}], shippingAddress, phone, note, paymentMethod, createdAt, updatedAt` cùng bốn trường tiền:
`subtotal` (tạm tính), `couponCode`, `couponPercent`, `discount` (tiền được giảm) và `total` (= `subtotal − discount`, số khách phải trả).
Đơn trả về cho **Admin** (`/api/admin/orders…`) có `id, customerId, customerName, email, phone, address, note, paymentMethod, status, items[{productId, productName, unitPrice, quantity, image}], createdAt, updatedAt` và cùng các trường tiền như trên
(lưu ý: Admin gọi là `address`, khách gọi là `shippingAddress`). Đơn không dùng mã có `couponCode: ""`, `couponPercent: 0`, `discount: 0`; đơn cũ tạo trước khi có tính năng mã giảm giá được hiển thị như vậy.

Mã đơn dạng `SP-YYMMDD-XXXX` (ví dụ `SP-261112-BEF7`). Trạng thái: `pending` → `confirmed` → `shipping` → `completed`; `pending`/`confirmed` có thể sang `cancelled`.

## Mã giảm giá (khách hàng)

Mã giảm **theo phần trăm** (1–90 %). Máy chủ là nơi duy nhất quyết định số tiền giảm: giao diện chỉ hiển thị kết quả, còn khi đặt hàng máy chủ luôn kiểm tra lại mã và tính lại tiền (khách có sửa dữ liệu gửi lên cũng vô tác dụng).

| Phương thức | Đường dẫn | Quyền | Mô tả |
|---|---|---|---|
| GET | `/api/coupons` | Customer | Các mã khách **đang có thể dùng** (đang bật, còn lượt, khách này chưa dùng nếu là mã 1 lần), nhiều % nhất trước. Mỗi mã: `{code, description, percent, minOrder, oncePerCustomer, remaining}` (`remaining` là số lượt còn lại, `null` nếu không giới hạn). |
| POST | `/api/coupons/validate` | Customer | `{code}` → áp mã thử lên **giỏ hàng hiện tại**: `{code, description, percent, minOrder, subtotal, discount, total}`. Không ghi dữ liệu gì. Giỏ trống → `409`. |

Quy tắc kiểm tra (theo thứ tự): mã không được để trống (`400`) → mã tồn tại (`404`) → đang bật (`409`) → còn lượt (`409`) → khách chưa dùng nếu là mã 1 lần (`409`) → tạm tính đạt mức đơn tối thiểu, thông báo cho biết còn thiếu bao nhiêu (`409`).
Mã được chuẩn hóa trước khi so sánh: bỏ khoảng trắng đầu/cuối và viết hoa (`welcome10` = `WELCOME10`).
Số tiền giảm = tạm tính × % / 100, làm tròn xuống đồng.
**Lượt dùng** của một mã là số đơn **chưa hủy** có dùng mã đó; vì vậy hủy đơn sẽ trả lại lượt (và trả lại quyền dùng cho khách ở mã 1 lần). Mã không có ngày hết hạn: muốn ngừng thì tắt mã.

## Trợ lý mua sắm

| Phương thức | Đường dẫn | Quyền | Mô tả |
|---|---|---|---|
| POST | `/api/ai/chat` | Customer | `{message}` → `{answer, products[], comparison, filters:{maxPrice, category}}`. Hiểu ngân sách ("dưới 2 triệu", "500k", "1,5tr"), tên danh mục, "rẻ nhất", "bán chạy", "so sánh". Luật trên dữ liệu cửa hàng, không gọi dịch vụ ngoài. |

## Quản trị (`Admin`)

Đăng nhập: `POST /api/admin/auth/login` (`{email, password}`), đăng xuất `POST /api/admin/auth/logout`, thông tin `GET /api/admin/auth/me`.

| Phương thức | Đường dẫn | Mô tả |
|---|---|---|
| GET | `/api/admin/dashboard` | Số liệu tổng quan: `revenue, todayRevenue, discountTotal, orderCount, todayOrders, pendingOrders, customerCount, productCount, lowStockCount, outOfStockCount, lowStockThreshold`, `revenueSeries` (14 ngày), `statusSeries`, `recentOrders`, `bestSellers`, `lowStock`. |
| GET | `/api/admin/reports/summary?from=YYYY-MM-DD&to=YYYY-MM-DD` | Báo cáo theo khoảng ngày: `revenue, discountTotal, couponOrders, orderCount, completedCount, averageOrderValue, customerCount, productCount, revenueSeries, statusSeries, categorySeries, topProducts, topCustomers`. Doanh thu chỉ tính đơn `completed` và là số tiền **sau giảm giá**; `discountTotal` là tổng tiền đã giảm trên các đơn đó, `couponOrders` là số đơn có dùng mã. |
| GET · POST | `/api/admin/products` | Danh sách (kể cả đang ẩn) · tạo sản phẩm. |
| GET · PUT · DELETE | `/api/admin/products/:id` | Xem · thay toàn bộ · xóa (gỡ khỏi giỏ của khách). |
| PATCH | `/api/admin/products/:id/stock` | `{stock}` |
| PATCH | `/api/admin/products/:id/visibility` | `{visible: true\|false}` |
| GET · POST | `/api/admin/categories` | Danh sách · tạo. |
| PUT · DELETE | `/api/admin/categories/:id` | Sửa · xóa (`409` nếu còn sản phẩm). |
| GET · POST | `/api/admin/coupons` | Danh sách mọi mã (kể cả đang tắt/hết lượt) kèm `usedCount`, `discountGiven` · tạo mã `{code, description?, percent, minOrder?, usageLimit?, oncePerCustomer?, active?}`. `minOrder` 0–10.000.000.000 (0 = mọi đơn), `usageLimit` 0–1.000.000 (0 = không giới hạn), `oncePerCustomer` mặc định `true`, `active` mặc định `true`. Trùng mã → `409`. |
| PUT | `/api/admin/coupons/:code` | Sửa các trường trên (không đổi được `code`). |
| PATCH | `/api/admin/coupons/:code/status` | `{active: true\|false}` – bật/tắt mã. |
| DELETE | `/api/admin/coupons/:code` | Xóa mã; `409` nếu đã từng có đơn dùng mã (hãy tắt mã thay vì xóa). |
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
`admin.product.created|updated|deleted|stock_updated|visibility_updated`, `admin.category.created|updated|deleted`, `admin.coupon.created|updated|status_changed|deleted`,
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

# Thử mã giảm giá lên giỏ hiện tại, rồi xem các mã đang dùng được
curl -s -X POST localhost:8080/api/coupons/validate -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' -d '{"code":"welcome10"}'
curl -s localhost:8080/api/coupons -H "Authorization: Bearer $TOKEN"
```
