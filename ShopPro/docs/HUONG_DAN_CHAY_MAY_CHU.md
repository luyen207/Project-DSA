# ShopPro – Hướng dẫn tự chạy máy chủ (dành cho thành viên nhóm)

Làm theo từ trên xuống, từng bước một. Cuối mỗi bước có dòng **"Thành công khi"** để bạn biết đã làm đúng chưa.
Lần đầu mất khoảng **15–25 phút** (phần lớn là chờ cài đặt). Những lần sau chỉ cần **2 lệnh**.

Bạn cần có:
* Máy Windows 10 (bản cập nhật gần đây) hoặc Windows 11, có Internet.
* File **`ShopPro.zip`** (bản đầy đủ) mà nhóm gửi, đã tải về thư mục *Downloads*.
  Các gói nhỏ như `ShopPro_Frontend.zip` hay `ShopPro_Backend_API.zip` **không đủ** để chạy cả trang web, hãy dùng bản đầy đủ.

> **Vì sao phải cài WSL?** Máy chủ ShopPro viết bằng C++ và được kiểm thử trên Linux. **WSL** là tính năng của Windows cho phép chạy Linux (Ubuntu) ngay trong Windows, không cần cài máy ảo.
> Bạn vẫn dùng trình duyệt Windows như bình thường để xem trang web.

Cách dán lệnh vào terminal Ubuntu: bấm **chuột phải** hoặc **Ctrl+Shift+V**. Dán cả khối nhiều dòng một lần cũng được (trừ những chỗ có ghi chú riêng).

---

## Bước 1. Cài WSL (chỉ làm một lần, bỏ qua nếu đã có Ubuntu)

1. Bấm Start, gõ **PowerShell**, bấm chuột phải vào kết quả → **Run as administrator**.
2. Gõ lệnh sau rồi Enter, chờ cài xong:

```powershell
wsl --install
```

3. **Khởi động lại máy** khi được yêu cầu.
4. Sau khi mở lại máy, cửa sổ **Ubuntu** có thể tự bật lên. Nếu không, bấm Start, gõ **Ubuntu** và mở nó. Lần đầu nó sẽ hỏi:
   * *Enter new UNIX username*: đặt tên bất kỳ, **chữ thường, không dấu, không khoảng trắng** (ví dụ `an`).
   * *New password*: đặt mật khẩu. **Gõ mật khẩu sẽ không hiện ký tự nào, đó là bình thường.** Gõ xong bấm Enter, gõ lại lần nữa để xác nhận. **Hãy nhớ mật khẩu này**, bước 2 sẽ dùng lại.
   * Nếu có câu hỏi về việc gửi số liệu ẩn danh, trả lời `y` hoặc `n` đều được.

**Thành công khi:** cửa sổ Ubuntu hiện một dòng có dạng `tenban@TENMAY:~$` và chờ bạn gõ lệnh.

---

## Bước 2. Cài công cụ biên dịch (chỉ làm một lần)

Dán lệnh dưới đây **một mình** (không dán kèm lệnh khác). Nó sẽ hỏi mật khẩu bạn đặt ở bước 1; gõ vào rồi Enter (vẫn không hiện ký tự):

```bash
sudo apt update && sudo apt install -y build-essential cmake libssl-dev unzip
```

Chờ vài phút đến khi dòng `tenban@TENMAY:~$` hiện lại.

**Thành công khi:** gõ `cmake --version` ra một dòng bắt đầu bằng `cmake version`.

---

## Bước 3. Giải nén dự án (chỉ làm một lần)

Dán nguyên khối này, **không cần sửa gì** (nó tự tìm file ZIP trong máy bạn):

```bash
cd ~
ZIP=$(find /mnt/c/Users /mnt/d /mnt/e -maxdepth 4 \( -name "ShopPro.zip" -o -name "ShopPro (*).zip" \) -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -1 | cut -d' ' -f2-)
echo "Tìm thấy: $ZIP"
```

* Nếu dòng `Tìm thấy:` có đường dẫn đến `ShopPro.zip` thì làm tiếp.
* Nếu dòng đó **để trống**: file ZIP chưa nằm ở thư mục Downloads (hoặc ổ D, E). Hãy chép nó vào Downloads rồi dán lại khối trên.
* Nếu Downloads có nhiều bản ZIP, lệnh sẽ chọn bản **mới nhất**. Nên xóa bản cũ cho khỏi nhầm.

Rồi giải nén:

```bash
unzip -o "$ZIP" -d ~
ls ~/ShopPro
```

> Đã từng giải nén ShopPro trước đó? Đổi tên thư mục cũ trước để khỏi trộn lẫn hai bản: `mv ~/ShopPro ~/ShopPro_cu`

**Thành công khi:** lệnh `ls` in ra `Backend  Dockerfile  Frontend  README.md  docs  scripts`.

---

## Bước 4. Biên dịch và kiểm thử (chỉ làm một lần)

```bash
cd ~/ShopPro
bash scripts/build.sh --test
```

Mất khoảng **1–3 phút**, màn hình chạy nhiều dòng là bình thường.

**Thành công khi:** gần cuối có dòng `100% tests passed, 0 tests failed out of 7` và dòng `Xong. Chạy máy chủ bằng: scripts/run.sh`.

---

## Bước 5. Chạy máy chủ

```bash
cd ~/ShopPro
bash scripts/run.sh
```

Terminal sẽ hiện một khung chữ có dòng **"ShopPro dang chay"**. Bây giờ mở trình duyệt **trên Windows** (Chrome, Edge…):

| Địa chỉ | Là gì |
|---|---|
| http://127.0.0.1:8080/ | Trang chọn giao diện |
| http://127.0.0.1:8080/shop/ | Cửa hàng (khách hàng) |
| http://127.0.0.1:8080/admin/ | Quản trị viên |

Tài khoản demo (trang đăng nhập có nút **Điền sẵn**):

| Vai trò | Email | Mật khẩu |
|---|---|---|
| Quản trị viên | `admin@shoppro.vn` | `Admin@12345` |
| Khách hàng | `khach@shoppro.vn` | `Demo@12345` |

**Thành công khi:** trang `/shop/` hiện danh sách sản phẩm và đăng nhập được.

Lưu ý:
* **Cửa sổ terminal phải để mở** suốt lúc dùng. Đóng cửa sổ là máy chủ tắt.
* Muốn **dừng** máy chủ: bấm **Ctrl+C** trong cửa sổ đó.
* Mỗi lần tắt máy chủ rồi bật lại, bạn phải **đăng nhập lại** (phiên đăng nhập chỉ lưu trong bộ nhớ). Đơn hàng và giỏ hàng thì vẫn còn.
* Để demo hai màn hình cùng lúc, mở hai cửa sổ trình duyệt đặt cạnh nhau: một cửa sổ vào `/admin/`, một cửa sổ vào `/shop/`.

---

## Những lần sau: chỉ 2 lệnh

Bấm Start, gõ **Ubuntu** để mở terminal, rồi:

```bash
cd ~/ShopPro
bash scripts/run.sh
```

Không cần cài hay biên dịch lại.

---

## Khi nhóm gửi bản mới hoặc bạn sửa code

| Tình huống | Cách làm |
|---|---|
| Sửa **giao diện** (HTML, CSS, JS trong `Frontend/`) | Không cần tắt máy chủ. Lưu file rồi bấm **F5** trên trình duyệt (nếu hình ảnh hoặc font chưa đổi thì **Ctrl+F5**). |
| Sửa **mã C++** trong `Backend/` | `Ctrl+C` → `bash scripts/build.sh` → `bash scripts/run.sh` |
| Nhận **`ShopPro.zip` mới** | `Ctrl+C`, rồi làm lại Bước 3 và Bước 4 (nhớ đổi tên thư mục cũ như ghi chú ở Bước 3), cuối cùng chạy lại Bước 5 |
| Muốn **về dữ liệu mẫu ban đầu** | `Ctrl+C`, rồi `bash scripts/reset-data.sh -y`. Lệnh này xóa các đơn và tài khoản bạn đã tự thử. |

Mở thư mục dự án bằng File Explorer của Windows để copy, sửa file (đứng trong terminal và gõ):

```bash
cd ~/ShopPro
explorer.exe .
```

Nếu máy bạn đã cài VS Code kèm tiện ích WSL thì gõ `code .` cũng mở được dự án để chỉnh sửa.

Mỗi người chạy máy chủ **trên máy của mình**, dữ liệu của ai người ấy giữ (nằm ở `Backend/data/runtime/`), nên thử đặt hàng, xóa dữ liệu… không ảnh hưởng tới bạn khác.

---

## Gặp lỗi thì xem ở đây

| Hiện tượng | Cách xử lý |
|---|---|
| `wsl --install` in ra một bảng trợ giúp dài thay vì cài | WSL đã có sẵn trên máy. Chạy `wsl --install -d Ubuntu` để cài Ubuntu. |
| Cài WSL báo lỗi liên quan đến ảo hóa (virtualization) | Máy cần bật tính năng ảo hóa trong BIOS/UEFI. Tên mục thường là *Virtualization Technology* hoặc *SVM Mode*, tùy hãng máy. |
| Gõ mật khẩu mà không thấy gì | Bình thường. Cứ gõ rồi Enter. |
| `sudo` báo `Sorry, try again` | Gõ sai mật khẩu Ubuntu. Gõ lại cẩn thận. |
| `unzip: command not found` | Bước 2 chưa chạy xong hoặc chưa chạy. Làm lại Bước 2. |
| Dòng `Tìm thấy:` để trống | File `ShopPro.zip` chưa ở Downloads (hoặc ổ D, E). Chép vào Downloads rồi dán lại khối lệnh. |
| `No such file or directory` khi chạy `scripts/...` | Đang đứng sai thư mục. Gõ `cd ~/ShopPro` rồi chạy lại. Gõ `ls` phải thấy `Backend`, `Frontend`, `scripts`. |
| `Permission denied` | Có thể bạn quên chữ `bash` ở đầu lệnh. Gõ đúng như hướng dẫn: `bash scripts/run.sh`. |
| `Không mở được cổng 8080 (đã có chương trình khác dùng cổng này?)` | Máy chủ đang chạy ở một cửa sổ khác. Tắt cửa sổ đó, hoặc đổi cổng: `bash scripts/run.sh --port 9000` rồi mở `http://127.0.0.1:9000/`. |
| Trình duyệt báo không kết nối được | Cửa sổ terminal có còn khung "ShopPro dang chay" không? Nếu không, chạy lại Bước 5. Thử thêm địa chỉ `http://localhost:8080/`. |
| Giao diện không đổi sau khi cập nhật | Bấm **Ctrl+F5**. Nếu vẫn không đổi, kiểm tra bạn đang chạy đúng thư mục (xem dòng `Frontend :` trong khung chữ lúc khởi động, nó ghi rõ máy chủ đang đọc thư mục nào). |
| Biên dịch báo thiếu `cmake` hoặc OpenSSL | Làm lại Bước 2, rồi chạy lại `bash scripts/build.sh`. |
| Trang trắng hoặc nút không bấm được | Bấm **F12** → thẻ **Console**, chụp màn hình lỗi màu đỏ gửi cho người phụ trách. |

Gặp lỗi khác: chụp **toàn bộ cửa sổ terminal** (thấy cả lệnh bạn gõ lẫn thông báo lỗi) gửi cho nhóm. Chỉ cần nói "bị lỗi" thì rất khó đoán nguyên nhân.

---

## Bảng lệnh nhanh

```bash
cd ~/ShopPro                          # vào thư mục dự án
bash scripts/build.sh --test          # biên dịch + kiểm thử (khi có mã C++ mới)
bash scripts/run.sh                   # chạy máy chủ  → http://127.0.0.1:8080/
# Ctrl+C                              # dừng máy chủ
bash scripts/reset-data.sh -y         # về dữ liệu mẫu ban đầu (khi máy chủ đang tắt)
bash scripts/run.sh --port 9000       # chạy ở cổng khác
```

---

## Dùng Linux hoặc macOS thì sao?

* **Ubuntu/Debian thật** (không qua WSL): bỏ Bước 1, làm Bước 2 trở đi. Chỗ tìm file ZIP, bạn chỉ cần `cd` tới nơi để file, hoặc chạy `unzip -o ShopPro.zip -d ~`.
* **macOS:** *chưa kiểm tra.* Cần cài Xcode Command Line Tools, CMake và OpenSSL (ví dụ bằng Homebrew), sau đó các lệnh `bash scripts/...` giống như trên. Nếu gặp lỗi, báo lại cho nhóm.
* **Windows chạy trực tiếp, không qua WSL** (`scripts\build.bat`, `scripts\run.bat`): *chưa kiểm tra.* Nhóm khuyên dùng WSL như hướng dẫn trên.

## Chưa kiểm tra

* Cách chạy trên macOS và trên Windows không qua WSL (như ghi ở trên).
* Truy cập từ điện thoại hoặc máy khác trong cùng mạng khi chạy trong WSL: chưa kiểm tra.
* Đưa lên tên miền công khai: xem `docs/TRIEN_KHAI_WEB.md`; bản này chưa được triển khai thật.

Xem thêm: `README.md` (tổng quan dự án), `Backend/README.md` (biên dịch chi tiết, tham số dòng lệnh), `docs/API.md` (danh sách REST API).
