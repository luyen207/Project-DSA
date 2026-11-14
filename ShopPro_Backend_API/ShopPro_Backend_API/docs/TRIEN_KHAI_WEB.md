# Đưa ShopPro lên web (tên miền công khai, HTTPS)

> **Trạng thái trung thực:** bản giao kèm tài liệu này **chưa được triển khai lên Internet** (môi trường phát triển không có mạng ra ngoài và không có tài khoản hosting).
> `Dockerfile` đã viết theo đúng cách chạy của chương trình nhưng **chưa được build thử** vì môi trường không có Docker.
> Phần chạy trực tiếp trên máy (`build/shoppro_api`) đã được kiểm thử đầy đủ. Hãy chạy thử Docker trên máy bạn trước khi đem đi thi/demo.

Một tiến trình `shoppro_api` phục vụ **cả ba thứ trên cùng một địa chỉ**, nên chỉ cần một dịch vụ và một tên miền:

| Đường dẫn | Nội dung |
|---|---|
| `https://ten-mien/` | Trang chọn giao diện (Khách hàng / Quản trị) |
| `https://ten-mien/shop/` | Giao diện khách hàng |
| `https://ten-mien/admin/` | Giao diện quản trị |
| `https://ten-mien/api/...` | REST API |

Vì cùng một nguồn gốc (origin) nên không cần cấu hình CORS. Muốn demo "đồng thời" hai giao diện: mở hai tab (hoặc hai cửa sổ), một tab `/shop/`, một tab `/admin/`.
Hai giao diện lưu phiên đăng nhập bằng hai khóa riêng nên đăng nhập cả hai cùng lúc không đè nhau.

## Biến môi trường dùng khi triển khai

| Biến | Giá trị khuyên dùng | Ghi chú |
|---|---|---|
| `HOST` | `0.0.0.0` | Mặc định `127.0.0.1` chỉ nhận kết nối nội bộ. Dockerfile đã đặt sẵn. |
| `PORT` | do nền tảng cấp | Render/Railway/Fly tự đặt; chương trình đọc biến này. |
| `TRUST_PROXY` | `1` | **Chỉ bật** khi đứng sau proxy/HTTPS của nền tảng, để giới hạn đăng nhập theo đúng IP khách (đọc `X-Forwarded-For`). Không bật khi mở thẳng ra Internet. |
| `SHOPPRO_DATA` | `/data/shoppro.json` | Tệp dữ liệu; thư mục chứa nó cũng chứa `activity-log.jsonl`. Phải nằm trên ổ đĩa bền vững nếu muốn giữ dữ liệu. |

## Cách 1 – Docker trên máy bạn (nên thử trước)

```bash
docker build -t shoppro .
docker run --rm -p 8080:8080 -v shoppro-data:/data shoppro
# mở http://localhost:8080/
```

`-v shoppro-data:/data` giữ dữ liệu khi khởi động lại container. Không có volume thì mỗi lần chạy lại quay về dữ liệu mẫu.

## Cách 2 – Nền tảng đám mây dùng Dockerfile (Render, Railway, Fly.io…)

Các bước chung (giao diện từng nền tảng có thể khác, hãy theo hướng dẫn hiện hành của họ):

1. Đưa thư mục `ShopPro/` lên một kho GitHub (riêng tư cũng được).
2. Tạo dịch vụ web mới từ kho đó, chọn kiểu **Docker** (nền tảng tự dùng `Dockerfile` ở thư mục gốc).
3. Đặt biến môi trường `TRUST_PROXY=1` (các biến còn lại đã có trong Dockerfile).
4. Thêm **ổ đĩa bền vững (persistent disk/volume)** gắn vào `/data` nếu muốn giữ đơn hàng và tài khoản mới sau mỗi lần triển khai lại.
   Một số gói miễn phí không có ổ đĩa: khi đó dữ liệu quay về bản mẫu mỗi lần dịch vụ khởi động lại – vẫn chấp nhận được cho buổi demo vì dữ liệu mẫu đã đầy đủ.
5. Đặt đường dẫn kiểm tra sức khỏe (health check) là `/api/health`.
6. Triển khai xong, nền tảng cấp địa chỉ HTTPS dạng `https://ten-dich-vu.<nền tảng>`; có thể gắn tên miền riêng trong phần *Custom Domain*.

Lưu ý: gói miễn phí thường "ngủ" khi lâu không có truy cập, lần mở đầu có thể chậm vài chục giây – hãy mở trước buổi demo.

## Cách 3 – Máy chủ riêng (VPS) + Caddy tự cấp HTTPS

Cần một VPS Linux và một tên miền đã trỏ bản ghi A về IP của VPS.

```bash
# Trên VPS (Ubuntu/Debian)
sudo apt install build-essential cmake libssl-dev caddy
git clone <kho-của-bạn> ShopPro && cd ShopPro
cmake -S Backend -B build -DCMAKE_BUILD_TYPE=Release -DSHOPPRO_BUILD_TESTS=OFF
cmake --build build --target shoppro_api --parallel
TRUST_PROXY=1 build/shoppro_api --port 8080 --quiet     # nên chạy bằng systemd, xem bên dưới
```

`/etc/caddy/Caddyfile` (Caddy tự lấy và gia hạn chứng chỉ HTTPS):

```
ten-mien.cua-ban.com {
    reverse_proxy 127.0.0.1:8080
}
```

Ví dụ `systemd` (`/etc/systemd/system/shoppro.service`):

```ini
[Unit]
Description=ShopPro
After=network.target

[Service]
WorkingDirectory=/home/ubuntu/ShopPro
Environment=TRUST_PROXY=1 PORT=8080
ExecStart=/home/ubuntu/ShopPro/build/shoppro_api --quiet
Restart=on-failure
User=ubuntu

[Install]
WantedBy=multi-user.target
```

Dùng `sudo systemctl enable --now shoppro`. Không mở cổng 8080 ra ngoài tường lửa; chỉ mở 80 và 443 cho Caddy.

## Trước khi mở công khai – bắt buộc xem

1. **Đổi hoặc xóa tài khoản demo.** Mật khẩu `Admin@12345` và `Demo@12345` được in sẵn trong khung "Tài khoản demo" ở hai trang đăng nhập và trong tài liệu, ai cũng biết.
   * Tạo Admin mới: `SHOPPRO_ADMIN_PASSWORD='…' shoppro_api --create-admin email@cua-ban.vn "Tên"`.
   * Khóa hoặc xóa các tài khoản mẫu: đăng nhập Admin → **Khách hàng** → khóa tài khoản `khach@shoppro.vn`; với Admin mẫu, sửa dữ liệu hoặc tạo lại `seed.json` bằng `Backend/data/tools/generate_seed.py` với mật khẩu khác.
   * Xóa dòng `DEMO` và khối `demo-box` trong `Frontend/admin/js/app.js` và `Frontend/shop/js/views/account.js`.
2. Chỉ bật `TRUST_PROXY=1` khi thật sự có proxy phía trước; nếu không, người dùng có thể giả `X-Forwarded-For` để né giới hạn đăng nhập.
3. Máy chủ **không tự có HTTPS**. Luôn đặt sau proxy/nền tảng có HTTPS, vì mật khẩu và token đi qua mạng.
4. Sao lưu định kỳ tệp `SHOPPRO_DATA` và `activity-log.jsonl` (chỉ cần chép tệp).
5. Dữ liệu là một tệp JSON nạp hết vào RAM; ứng dụng phù hợp demo/đồ án, không dành cho tải lớn. Xem [Giới hạn đã biết](../Backend/README.md#giới-hạn-đã-biết).

## Kiểm tra sau khi triển khai

```bash
curl -s https://ten-mien/api/health        # {"status":"ok",...}
curl -sI https://ten-mien/shop/ | head -1  # HTTP/2 200
curl -sI https://ten-mien/admin/ | head -1
```

Sau đó mở `/shop/`, đăng nhập khách demo, thêm sản phẩm vào giỏ, đặt hàng; mở `/admin/` ở tab khác, đăng nhập Admin và xác nhận đơn vừa đặt (bảng điều khiển tự cập nhật số đơn chờ sau tối đa 15 giây).
