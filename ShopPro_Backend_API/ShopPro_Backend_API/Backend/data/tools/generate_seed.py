#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Sinh du lieu mau ShopPro (xac dinh - cung ma ngau nhien cho cung ket qua).

  python3 Backend/data/tools/generate_seed.py

Tao:  Backend/data/seed.json            (san pham, danh muc, tai khoan, don hang, cai dat)
      Backend/data/seed-activity.jsonl  (nhat ky hoat dong mau)
      Frontend/img/products/<id>.svg    (anh san pham ve bang SVG, chay duoc khi khong co Internet)

Mat khau tai khoan mau CHI duoc luu dang bam PBKDF2-HMAC-SHA256 (khong co mat khau thuong trong tep).
Mat khau demo: xem README (Admin: Admin@12345, Khach: Demo@12345).
"""
import hashlib
import json
import os
import random
import secrets
import sys
import unicodedata
from datetime import datetime, timedelta, timezone

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from icons import icon  # noqa: E402

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BACKEND_DATA = os.path.join(ROOT, "Backend", "data")
IMG_DIR = os.path.join(ROOT, "Frontend", "img", "products")
REFERENCE = datetime(2026, 11, 14, 9, 0, 0, tzinfo=timezone.utc)   # moc "hom nay" cua du lieu mau
ITERATIONS = 600000
rnd = random.Random(20261114)


def iso(dt):
    return dt.strftime("%Y-%m-%dT%H:%M:%S.000Z")


def fold(text):
    text = text.replace("đ", "d").replace("Đ", "D")
    return "".join(c for c in unicodedata.normalize("NFD", text) if unicodedata.category(c) != "Mn")


# ---------------------------------------------------------------- danh muc
CATEGORIES = [
    # (ten, mo ta, mau nen, mau dam, mau nhat)  -- tuyet doi khong dung mau tim
    ("Điện tử & Phụ kiện", "Tai nghe, loa, đồng hồ thông minh và phụ kiện công nghệ", "#1E6FD9", "#0B3E8A", "#CFE3FF"),
    ("Thời trang", "Quần áo, giày dép và phụ kiện hằng ngày", "#E4572E", "#8C2A10", "#FFD9CC"),
    ("Nhà cửa & Đời sống", "Đồ dùng tiện ích cho căn bếp và không gian sống", "#0E9F8E", "#05564D", "#C8F1EA"),
    ("Sách & Văn phòng phẩm", "Sách chuyên ngành, sổ tay và dụng cụ học tập", "#D98A00", "#6B4300", "#FFEFC2"),
    ("Thể thao & Dã ngoại", "Dụng cụ tập luyện, cắm trại và vận động ngoài trời", "#2E9E4F", "#135A28", "#D3F2DC"),
    ("Làm đẹp & Chăm sóc", "Chăm sóc da, tóc và cơ thể mỗi ngày", "#1AA6C9", "#0A5870", "#CDEFF8"),
    ("Mẹ & Bé", "Sản phẩm an toàn cho mẹ và em bé", "#6FA61F", "#35550B", "#E6F5C9"),
    ("Thực phẩm & Đồ uống", "Cà phê, trà, đồ ăn vặt và thực phẩm khô", "#B5651D", "#5E3510", "#F2DEC8"),
]

# (ten, danh muc 1-8, gia, ton kho, bieu tuong, mo ta, noi bat, ban chay)
P = [
    ("Tai nghe Bluetooth chống ồn SoundWave NC700", 1, 1890000, 34, "headphones", "Chống ồn chủ động 40 dB, Bluetooth 5.3, pin 45 giờ, sạc nhanh 10 phút nghe 5 giờ. Đệm tai memory foam êm ái.", 1, 1),
    ("Loa Bluetooth di động BassBox Mini 20W", 1, 790000, 58, "speaker", "Âm bass dày, chống nước IPX7, pin 12 giờ, ghép đôi hai loa tạo âm thanh nổi.", 0, 0),
    ("Đồng hồ thông minh FitBand S5", 1, 1290000, 41, "watch", "Màn AMOLED 1.43 inch, đo nhịp tim và SpO2, 100 chế độ thể thao, pin 14 ngày.", 1, 0),
    ("Bàn phím cơ không dây KeyCraft K84", 1, 1150000, 27, "keyboard", "Layout 84 phím, switch red, kết nối 3 chế độ, đèn nền trắng, pin 4000 mAh.", 0, 0),
    ("Chuột không dây ErgoGlide M3", 1, 349000, 96, "mouse", "Thiết kế công thái học, cảm biến 4000 DPI, kết nối 2.4G và Bluetooth, pin 6 tháng.", 0, 1),
    ("Sạc dự phòng PowerCell 20.000mAh 65W", 1, 899000, 7, "charger", "Sạc nhanh PD 65W cho cả laptop, hai cổng USB-C và một cổng USB-A, màn hình hiển thị % pin.", 0, 0),
    ("Củ sạc nhanh GaN 65W 3 cổng", 1, 459000, 73, "charger", "Công nghệ GaN nhỏ gọn, 2 cổng USB-C và 1 cổng USB-A, tự phân bổ công suất.", 0, 0),
    ("Ốp lưng chống sốc ClearGuard trong suốt", 1, 129000, 220, "phone", "Viền chống sốc 4 góc, không ố vàng, cho phép sạc không dây.", 0, 0),

    ("Áo thun cotton nam Basic Fit", 2, 199000, 180, "tshirt", "100% cotton 220gsm dày dặn, thấm hút tốt, form regular dễ phối đồ.", 0, 1),
    ("Áo sơ mi nữ linen tay lỡ", 2, 349000, 64, "tshirt", "Vải linen thoáng mát, tay lỡ thoải mái, phù hợp đi làm và dạo phố.", 1, 0),
    ("Quần jean nam slim-fit co giãn", 2, 499000, 52, "pants", "Vải denim co giãn 2 chiều, form slim, màu xanh đậm bền màu sau nhiều lần giặt.", 0, 0),
    ("Giày sneaker nữ trắng Urban Step", 2, 690000, 38, "shoe", "Da tổng hợp dễ lau chùi, đế cao su êm, dễ phối với mọi trang phục.", 1, 0),
    ("Giày chạy bộ nam AirRun 2", 2, 1150000, 29, "shoe", "Đế đệm khí nhẹ 240g, lưới thoáng khí, độ nảy cao cho chạy đường dài.", 0, 1),
    ("Balo laptop chống nước 15.6 inch", 2, 559000, 45, "bag", "Ngăn chống sốc laptop, cổng sạc USB ngoài, khóa chống trộm, vải chống thấm.", 0, 1),
    ("Túi tote vải canvas dày", 2, 159000, 120, "bag", "Vải canvas 12oz bền chắc, có ngăn khóa kéo bên trong, in logo thân thiện môi trường.", 0, 0),
    ("Mũ lưỡi trai thêu chữ unisex", 2, 119000, 0, "hat", "Cotton mềm, khóa điều chỉnh phía sau, thêu chữ nổi sắc nét.", 0, 0),

    ("Bình giữ nhiệt inox 600ml", 3, 359000, 88, "bottle", "Inox 304 giữ nóng 12 giờ, giữ lạnh 24 giờ, nắp kín không rò rỉ.", 1, 1),
    ("Ly sứ cách nhiệt 350ml có nắp", 3, 129000, 140, "mug", "Sứ cao cấp hai lớp, giữ nhiệt tốt, có nắp đậy và muỗng khuấy.", 0, 0),
    ("Đèn bàn LED chống cận 3 chế độ", 3, 429000, 33, "lamp", "Ánh sáng không nhấp nháy, 3 mức màu, 5 độ sáng, cổng sạc USB.", 1, 0),
    ("Chảo chống dính vân đá 26cm", 3, 389000, 47, "pan", "Lớp chống dính vân đá, dùng được trên bếp từ, tay cầm cách nhiệt.", 0, 0),
    ("Bộ ga gối cotton 1m6 (4 món)", 3, 749000, 22, "bed", "Cotton 100% mềm mịn, không xù lông, kèm 2 vỏ gối và 1 ga phủ.", 0, 0),
    ("Hộp cơm giữ nhiệt 2 ngăn", 3, 249000, 61, "box", "Giữ nhiệt 6 giờ, hai ngăn riêng biệt, dùng được trong lò vi sóng (tháo ruột).", 0, 0),
    ("Máy xay sinh tố mini 400ml", 3, 590000, 26, "blender", "Công suất 300W, 6 lưỡi dao inox, cốc xay mang đi tiện lợi, sạc USB.", 0, 0),
    ("Thảm lau chân thấm hút chống trượt", 3, 89000, 9, "mat", "Sợi microfiber thấm nước nhanh, đế cao su chống trượt, giặt máy được.", 0, 0),

    ("Sổ tay lò xo A5 giấy kem 200 trang", 4, 79000, 300, "notebook", "Giấy kem 100gsm chống lem mực, gáy lò xo mở phẳng 360 độ.", 0, 0),
    ("Bút gel 0.5mm hộp 12 cây", 4, 69000, 410, "pen", "Mực khô nhanh, nét đều 0.5mm, thân cầm êm tay, màu mực đen.", 0, 1),
    ("Lập trình C++ từ cơ bản đến nâng cao", 4, 225000, 44, "book", "Giáo trình C++17 với bài tập thực hành, OOP, STL và quản lý bộ nhớ.", 1, 0),
    ("Giáo trình Cấu trúc dữ liệu và Giải thuật", 4, 189000, 52, "book", "Danh sách liên kết, ngăn xếp, hàng đợi, cây và đồ thị kèm bài tập có lời giải.", 0, 1),
    ("Bộ bút highlight 6 màu pastel", 4, 59000, 150, "pen", "Đầu bút vát bản to, mực nước không thấm sang mặt sau trang giấy.", 0, 0),
    ("Kệ sách mini để bàn 2 tầng", 4, 289000, 31, "shelf", "Gỗ MDF phủ melamine chống ẩm, lắp ráp dễ dàng, tải trọng 10 kg.", 0, 0),
    ("Máy tính khoa học 570 chức năng", 4, 349000, 58, "calculator", "Màn hình 2 dòng, giải phương trình, ma trận, thống kê; được phép dùng trong phòng thi.", 0, 0),
    ("Giấy A4 70gsm (ream 500 tờ)", 4, 82000, 500, "paper", "Độ trắng cao, ít bụi, chạy tốt trên máy in laser và máy photocopy.", 0, 0),

    ("Thảm tập yoga TPE 6mm", 5, 249000, 70, "yoga", "Chất liệu TPE thân thiện môi trường, hai lớp chống trượt, kèm dây buộc.", 0, 0),
    ("Tạ tay bọc cao su 5kg (cặp)", 5, 399000, 40, "dumbbell", "Lõi gang bọc cao su chống ồn và trầy sàn, tay cầm có rãnh chống trơn.", 0, 0),
    ("Dây nhảy tốc độ có bộ đếm", 5, 99000, 130, "rope", "Dây cáp thép bọc nhựa, tay cầm có bộ đếm vòng, chiều dài điều chỉnh.", 0, 0),
    ("Lều cắm trại 2 người chống thấm", 5, 1290000, 14, "tent", "Hai lớp chống thấm 3000mm, khung sợi thủy tinh, lắp trong 5 phút, nặng 2.4 kg.", 1, 0),
    ("Bình nước thể thao 1 lít", 5, 149000, 110, "bottle", "Nhựa Tritan không BPA, có vạch chia dung tích và dây đeo.", 0, 0),
    ("Găng tay tập gym chống trượt", 5, 139000, 82, "dumbbell", "Lòng bàn tay đệm silicon chống trượt, cổ tay có quấn bảo vệ.", 0, 0),
    ("Bóng đá size 5 thi đấu", 5, 299000, 63, "ball", "Da PU may máy bền bỉ, bóng ruột bơm giữ hơi tốt, đạt chuẩn thi đấu phong trào.", 0, 0),
    ("Vợt cầu lông carbon (bộ 2 cây)", 5, 590000, 36, "racket", "Khung carbon nhẹ 85g, kèm bao đựng và 2 cuốn cán, phù hợp người mới chơi.", 0, 0),

    ("Sữa rửa mặt dịu nhẹ 150ml", 6, 169000, 160, "pump", "pH 5.5 dịu nhẹ, làm sạch bụi bẩn không gây khô da, dùng được cho da nhạy cảm.", 0, 0),
    ("Kem chống nắng SPF50+ PA++++ 50ml", 6, 249000, 92, "tube", "Kết cấu mỏng nhẹ không bết dính, không để lại vệt trắng, kháng nước.", 1, 1),
    ("Serum Vitamin C 30ml", 6, 329000, 55, "dropper", "Vitamin C 15% ổn định, hỗ trợ làm sáng da và mờ thâm, dạng dropper tiện dụng.", 0, 0),
    ("Dầu gội thảo dược 480ml", 6, 189000, 84, "pump", "Chiết xuất bồ kết và gừng, giảm gàu và rụng tóc, hương thơm tự nhiên.", 0, 0),
    ("Son dưỡng môi có màu", 6, 99000, 200, "tube", "Dưỡng ẩm cùng bơ hạt mỡ, lên màu nhẹ tự nhiên, không chứa chì.", 0, 0),
    ("Máy sấy tóc ion âm 1800W", 6, 549000, 31, "dryer", "Công nghệ ion âm giảm xơ rối, 3 mức nhiệt, kèm đầu tạo kiểu.", 0, 0),
    ("Bộ chăm sóc da mini 4 món", 6, 499000, 39, "jar", "Gồm sữa rửa mặt, toner, serum và kem dưỡng dung tích du lịch.", 0, 0),
    ("Xịt thơm body mist 100ml", 6, 219000, 3, "dropper", "Hương hoa cỏ nhẹ nhàng, lưu hương 4–6 giờ, chai xịt sương mịn.", 0, 0),

    ("Bình sữa PP cổ rộng 240ml", 7, 159000, 95, "babybottle", "Nhựa PP an toàn không BPA, núm ti silicon mềm, van chống sặc.", 0, 0),
    ("Tã quần size L (54 miếng)", 7, 329000, 66, "box", "Thấm hút nhanh, lớp bề mặt khô thoáng, co giãn 360 độ.", 0, 0),
    ("Khăn sữa sợi tre (5 cái)", 7, 119000, 140, "towel", "Sợi tre kháng khuẩn tự nhiên, mềm mịn cho làn da nhạy cảm của bé.", 0, 0),
    ("Xe đẩy gấp gọn đa năng", 7, 2490000, 8, "stroller", "Gấp gọn bằng một tay, mái che chống nắng UPF50+, ghế ngả 3 tư thế.", 1, 0),
    ("Bộ xếp hình sáng tạo 120 chi tiết", 7, 259000, 72, "blocks", "Nhựa ABS an toàn, bo tròn các cạnh, phát triển tư duy và kỹ năng vận động.", 0, 1),
    ("Gấu bông Teddy 50cm", 7, 189000, 58, "teddy", "Bông PP mềm mịn, lông ngắn không xù, có thể giặt máy.", 0, 0),
    ("Sữa tắm gội trẻ em 500ml", 7, 149000, 77, "pump", "Công thức không cay mắt, chiết xuất yến mạch, an toàn cho da bé.", 0, 0),
    ("Ghế ăn dặm tăng giảm chiều cao", 7, 890000, 19, "chair", "Khung nhựa ABS chắc chắn, khay ăn tháo rời, đai an toàn 5 điểm.", 0, 0),

    ("Cà phê rang xay nguyên chất 500g", 8, 189000, 120, "pouch", "Phối trộn Robusta – Arabica, rang vừa, đậm vị hậu ngọt, van một chiều giữ hương.", 1, 1),
    ("Trà ô long túi lọc (hộp 50 gói)", 8, 129000, 95, "tea", "Trà ô long cao nguyên, hương thơm dịu, túi lọc nilon tiện pha.", 0, 0),
    ("Mật ong rừng nguyên chất 500ml", 8, 259000, 45, "jar", "Thu hoạch từ rừng Tây Nguyên, không pha đường, có phiếu kiểm nghiệm.", 0, 0),
    ("Hạt điều rang muối 500g", 8, 179000, 80, "pouch", "Hạt điều Bình Phước loại lớn, rang củi giòn thơm, đóng gói hút chân không.", 0, 0),
    ("Mì Ý kèm sốt cà chua (set 2 phần)", 8, 99000, 108, "box", "Mì Ý bột lúa mì cứng 500g kèm sốt cà chua basil ăn liền.", 0, 0),
    ("Bánh quy bơ hộp thiếc 400g", 8, 149000, 52, "box", "Bơ lạt nhập khẩu, giòn tan, hộp thiếc dùng lại làm quà tặng.", 0, 0),
    ("Nước ép trái cây ép lạnh (6 chai)", 8, 219000, 36, "bottle", "Ép lạnh nguyên chất 330ml, không đường, không chất bảo quản, hạn dùng 14 ngày.", 0, 0),
    ("Granola ngũ cốc hạt 450g", 8, 139000, 0, "pouch", "Yến mạch nướng mật ong cùng hạnh nhân, óc chó và nam việt quất sấy.", 0, 0),
]

# ---------------------------------------------------------------- khach hang
CUSTOMERS = [
    ("Khách Demo", "khach@shoppro.vn", "0901234567", "12 Nguyễn Huệ, Phường Bến Nghé, Quận 1, TP. Hồ Chí Minh", "active"),
    ("Nguyễn Minh Anh", "minhanh@example.com", "0912345678", "45 Lê Lợi, Phường Bến Thành, Quận 1, TP. Hồ Chí Minh", "active"),
    ("Trần Quốc Bảo", "quocbao@example.com", "0923456789", "88 Trần Thái Tông, Phường Dịch Vọng, Cầu Giấy, Hà Nội", "active"),
    ("Lê Thị Thu Hà", "thuha@example.com", "0934567890", "21 Bạch Đằng, Phường Hải Châu 1, Quận Hải Châu, Đà Nẵng", "active"),
    ("Phạm Đức Anh", "ducanh@example.com", "0945678901", "150 Đường 30/4, Phường Xuân Khánh, Quận Ninh Kiều, Cần Thơ", "active"),
    ("Võ Ngọc Lan", "ngoclan@example.com", "0956789012", "7 Võ Văn Ngân, Phường Linh Chiểu, TP. Thủ Đức, TP. Hồ Chí Minh", "active"),
    ("Đặng Hoàng Nam", "hoangnam@example.com", "0967890123", "32 Quang Trung, Phường Quang Trung, Quận Hà Đông, Hà Nội", "active"),
    ("Bùi Thanh Tùng", "thanhtung@example.com", "0978901234", "19 Phạm Văn Thuận, Phường Tân Tiến, TP. Biên Hòa, Đồng Nai", "active"),
    ("Hoàng Mai Phương", "maiphuong@example.com", "0989012345", "64 Trần Phú, Phường Lộc Thọ, TP. Nha Trang, Khánh Hòa", "active"),
    ("Ngô Gia Huy", "giahuy@example.com", "0990123456", "5 Lê Quý Đôn, Phường Phú Hội, TP. Huế, Thừa Thiên Huế", "active"),
    ("Đỗ Khánh Linh", "khanhlinh@example.com", "0902345671", "101 Lạch Tray, Phường Đằng Giang, Quận Ngô Quyền, Hải Phòng", "active"),
    ("Dương Văn Khoa", "vankhoa@example.com", "0913456782", "27 Trương Công Định, Phường 3, TP. Vũng Tàu, Bà Rịa - Vũng Tàu", "locked"),
    ("Lý Bích Ngọc", "bichngoc@example.com", "0924567893", "300 Nguyễn Thị Thập, Phường Tân Phong, Quận 7, TP. Hồ Chí Minh", "active"),
    ("Phan Tuấn Kiệt", "tuankiet@example.com", "0935678904", "15 Nguyễn Huệ, Phường Lê Lợi, TP. Quy Nhơn, Bình Định", "active"),
    ("Trịnh Hải Yến", "haiyen@example.com", "0946789015", "9 Hoàng Văn Thụ, Phường 9, TP. Đà Lạt, Lâm Đồng", "active"),
]

NOTES = ["", "", "", "", "Giao giờ hành chính giúp mình nhé.", "Gọi trước khi giao 15 phút.", "Để hàng ở bảo vệ nếu mình vắng.", "Đóng gói kỹ giúp mình, cảm ơn shop!", "Xuất hóa đơn công ty theo email."]


def hash_password(password):
    salt = secrets.token_hex(16)
    digest = hashlib.pbkdf2_hmac("sha256", password.encode(), salt.encode(), ITERATIONS).hex()
    return salt, digest


def main():
    categories = [{"id": i + 1, "name": c[0], "description": c[1]} for i, c in enumerate(CATEGORIES)]

    # ---- san pham + anh ----
    os.makedirs(IMG_DIR, exist_ok=True)
    products = []
    for i, (name, cat, price, stock, ic, desc, featured, best) in enumerate(P):
        pid = 101 + i
        base, dark, light = CATEGORIES[cat - 1][2], CATEGORIES[cat - 1][3], CATEGORIES[cat - 1][4]
        svg = (
            '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 400 400" role="img" aria-label="%s">'
            '<defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="%s"/><stop offset="1" stop-color="%s"/></linearGradient></defs>'
            '<rect width="400" height="400" fill="url(#g)"/>'
            '<circle cx="330" cy="70" r="130" fill="#fff" opacity=".10"/><circle cx="50" cy="370" r="150" fill="#fff" opacity=".08"/>'
            '<ellipse cx="200" cy="346" rx="100" ry="12" fill="#000" opacity=".16"/>'
            '<g transform="translate(60 48) scale(1.4)">%s</g></svg>'
        ) % (name.replace('"', ""), base, dark, icon(ic, "#FFFFFF", dark, light))
        with open(os.path.join(IMG_DIR, f"{pid}.svg"), "w", encoding="utf-8") as fh:
            fh.write(svg)
        created = REFERENCE - timedelta(days=rnd.randint(20, 120), hours=rnd.randint(0, 20))
        products.append({
            "id": pid, "name": name, "categoryId": cat, "price": price, "stock": stock,
            "description": desc, "image": f"/img/products/{pid}.svg", "visible": True,
            "featured": bool(featured), "bestSelling": bool(best), "createdAt": iso(created), "updatedAt": iso(created),
        })
    # 1 san pham dang bi an de demo chuc nang an/hien
    hidden = {"id": 165, "name": "Tai nghe gaming có dây (ngừng kinh doanh)", "categoryId": 1, "price": 520000, "stock": 12,
              "description": "Sản phẩm đã ngừng kinh doanh, đang ẩn khỏi cửa hàng.", "image": "/img/products/101.svg",
              "visible": False, "featured": False, "bestSelling": False,
              "createdAt": iso(REFERENCE - timedelta(days=150)), "updatedAt": iso(REFERENCE - timedelta(days=30))}
    products.append(hidden)
    pmap = {p["id"]: p for p in products}

    # ---- tai khoan ----
    users = []
    pw_cache = {}
    def account(uid, name, email, phone, address, role, status, joined, password):
        salt, digest = hash_password(password)
        users.append({"id": uid, "name": name, "email": email, "phone": phone, "address": address, "role": role,
                      "status": status, "joinedAt": joined, "passwordSalt": salt, "passwordHash": digest,
                      "passwordIterations": ITERATIONS})
    account("adm-0001", "Quản trị viên ShopPro", "admin@shoppro.vn", "1900 1234", "", "admin", "active",
            (REFERENCE - timedelta(days=200)).strftime("%Y-%m-%d"), "Admin@12345")
    customer_ids = []
    for i, (name, email, phone, addr, status) in enumerate(CUSTOMERS):
        uid = f"cus-{1001 + i}"
        customer_ids.append(uid)
        account(uid, name, email, phone, addr, "customer", status,
                (REFERENCE - timedelta(days=rnd.randint(25, 170))).strftime("%Y-%m-%d"), "Demo@12345")
    umap = {u["id"]: u for u in users}

    # ---- don hang ----
    visible_products = [p for p in products if p["visible"]]
    orders = []
    used_ids = set()
    activity = []
    log_n = [0]

    def log(dt, action, actor, role, ttype, tid, result="success", details=None, ip=None):
        log_n[0] += 1
        entry = {"id": "log-%012x" % (0xA17000000000 + log_n[0] * 7919), "createdAt": iso(dt), "action": action,
                 "actorId": actor, "actorRole": role, "targetType": ttype, "targetId": tid, "result": result}
        if details:
            entry["details"] = details
        if ip:
            entry["ip"] = ip
        activity.append(entry)

    active_customers = [u for u in customer_ids if umap[u]["status"] == "active"]
    for n in range(64):
        # ngay dat: day dac hon o gan hien tai
        age_days = int(rnd.triangular(0, 45, 6))
        created = REFERENCE - timedelta(days=age_days, hours=rnd.randint(0, 9), minutes=rnd.randint(0, 59))
        if age_days == 0:
            created = REFERENCE - timedelta(minutes=rnd.randint(20, 520))
        uid = "cus-1001" if n < 4 else rnd.choice(customer_ids if age_days > 20 else active_customers)
        u = umap[uid]
        picks = rnd.sample(visible_products, rnd.choice([1, 1, 2, 2, 3, 4]))
        items, total = [], 0
        for p in picks:
            qty = rnd.choice([1, 1, 1, 2, 2, 3]) if p["price"] < 400000 else rnd.choice([1, 1, 1, 2])
            items.append({"productId": p["id"], "productName": p["name"], "image": p["image"],
                          "unitPrice": p["price"], "quantity": qty})
            total += p["price"] * qty
        # trang thai theo do tuoi don
        r = rnd.random()
        if age_days >= 10:
            status = "completed" if r < 0.90 else "cancelled"
        elif age_days >= 4:
            status = "completed" if r < 0.38 else "shipping" if r < 0.68 else "confirmed" if r < 0.90 else "cancelled"
        elif age_days >= 1:
            status = "shipping" if r < 0.25 else "confirmed" if r < 0.60 else "pending" if r < 0.94 else "cancelled"
        else:
            status = "pending" if r < 0.65 else "confirmed"
        stamp = created.strftime("%y%m%d")
        while True:
            oid = f"SP-{stamp}-{secrets.token_hex(2).upper()}"
            if oid not in used_ids:
                used_ids.add(oid)
                break
        # lich su cap nhat
        flow = {"pending": [], "confirmed": ["confirmed"], "shipping": ["confirmed", "shipping"],
                "completed": ["confirmed", "shipping", "completed"], "cancelled": ["cancelled"]}[status]
        t = created
        transitions = []
        prev = "pending"
        for step in flow:
            t = t + timedelta(hours=rnd.randint(2, 20), minutes=rnd.randint(0, 59))
            if t > REFERENCE:
                t = REFERENCE - timedelta(minutes=rnd.randint(1, 30))
            transitions.append((t, prev, step))
            prev = step
        updated = t if flow else created
        order = {"id": oid, "customerId": uid, "customerName": u["name"], "email": u["email"], "phone": u["phone"],
                 "shippingAddress": u["address"], "note": rnd.choice(NOTES), "paymentMethod": rnd.choice(["cod", "cod", "bank"]),
                 "createdAt": iso(created), "updatedAt": iso(updated), "status": status, "items": items,
                 "stockReserved": status != "cancelled", "total": total}
        orders.append(order)
        log(created, "order.placed", uid, "customer", "order", oid, details={"total": total, "itemCount": len(items), "paymentMethod": order["paymentMethod"]}, ip=f"113.161.{rnd.randint(1, 250)}.{rnd.randint(1, 250)}")
        for (tt, a, b) in transitions:
            log(tt, "admin.order.status_changed", "adm-0001", "admin", "order", oid, details={"from": a, "to": b}, ip="127.0.0.1")
    orders.sort(key=lambda o: o["createdAt"])

    # ---- cac su kien khac cho nhat ky ----
    for d in range(0, 30):
        day = REFERENCE - timedelta(days=d)
        log(day.replace(hour=8, minute=rnd.randint(0, 40)), "auth.login.succeeded", "adm-0001", "admin", "user", "adm-0001", ip="127.0.0.1")
        for uid in rnd.sample(active_customers, rnd.randint(1, 4)):
            log(day.replace(hour=rnd.randint(9, 22), minute=rnd.randint(0, 59)), "auth.login.succeeded", uid, "customer", "user", uid, ip=f"14.{rnd.randint(160, 190)}.{rnd.randint(1, 250)}.{rnd.randint(1, 250)}")
    for d in (2, 5, 9, 16):
        day = REFERENCE - timedelta(days=d)
        log(day.replace(hour=23, minute=12), "auth.login.failed", "", "customer", "user", "", "failure", {"email": "unknown@example.com"}, ip="103.77.12.9")
    for k in range(3):
        log((REFERENCE - timedelta(days=5)).replace(hour=23, minute=14 + k), "auth.login.failed", "", "customer", "user", "", "failure", {"email": "unknown@example.com"}, ip="103.77.12.9")
    log((REFERENCE - timedelta(days=5)).replace(hour=23, minute=20), "auth.login.blocked", "", "customer", "user", "", "blocked", {"email": "unknown@example.com"}, ip="103.77.12.9")
    for uid in customer_ids[1:]:
        joined = datetime.strptime(umap[uid]["joinedAt"], "%Y-%m-%d").replace(hour=10, tzinfo=timezone.utc)
        log(joined, "auth.registered", uid, "customer", "user", uid, ip=f"27.{rnd.randint(60, 90)}.{rnd.randint(1, 250)}.{rnd.randint(1, 250)}")
    for pid, action, det in [(104, "admin.product.stock_updated", {"before": 20, "after": 27}),
                             (106, "admin.product.stock_updated", {"before": 15, "after": 7}),
                             (136, "admin.product.updated", {"changed": ["price", "description"]}),
                             (165, "admin.product.visibility_updated", {"visible": False}),
                             (157, "admin.product.stock_updated", {"before": 30, "after": 3})]:
        log(REFERENCE - timedelta(days=rnd.randint(1, 20), hours=rnd.randint(0, 6)), action, "adm-0001", "admin", "product", str(pid), details=det, ip="127.0.0.1")
    log(REFERENCE - timedelta(days=21), "admin.settings.updated", "adm-0001", "admin", "settings", "store", ip="127.0.0.1")
    log(REFERENCE - timedelta(days=19), "admin.category.updated", "adm-0001", "admin", "category", "3", details={"name": "Nhà cửa & Đời sống"}, ip="127.0.0.1")
    log(REFERENCE - timedelta(days=12), "admin.customer.status_changed", "adm-0001", "admin", "user", "cus-1012", details={"status": "locked"}, ip="127.0.0.1")
    activity.sort(key=lambda e: e["createdAt"])

    # ---- gio hang mau cho tai khoan demo ----
    demo_cart = [{"productId": 136, "quantity": 1}, {"productId": 102, "quantity": 2}, {"productId": 126, "quantity": 1}, {"productId": 109, "quantity": 3}]
    for line in demo_cart:
        assert line["productId"] in pmap and pmap[line["productId"]]["stock"] >= line["quantity"]

    seed = {
        "meta": {"referenceTime": iso(REFERENCE), "note": "Du lieu mau - tai tao bang Backend/data/tools/generate_seed.py"},
        "categories": categories, "products": products, "users": users, "orders": orders,
        "carts": {"cus-1001": demo_cart},
        "settings": {"storeName": "ShopPro", "supportEmail": "support@shoppro.vn", "supportPhone": "1900 1234",
                     "lowStockThreshold": 10, "currency": "VND"},
    }
    with open(os.path.join(BACKEND_DATA, "seed.json"), "w", encoding="utf-8") as fh:
        json.dump(seed, fh, ensure_ascii=False, indent=2)
        fh.write("\n")
    with open(os.path.join(BACKEND_DATA, "seed-activity.jsonl"), "w", encoding="utf-8") as fh:
        for e in activity:
            fh.write(json.dumps(e, ensure_ascii=False, separators=(",", ":")) + "\n")
    by_status = {}
    for o in orders:
        by_status[o["status"]] = by_status.get(o["status"], 0) + 1
    print("san pham:", len(products), "| danh muc:", len(categories), "| tai khoan:", len(users),
          "| don hang:", len(orders), by_status, "| nhat ky:", len(activity))


if __name__ == "__main__":
    main()
