// Kiem thu REST API (khong qua mang): goi thang Router voi du lieu mau that.
// Gom ca cac hoi quy cho loi da tim thay luc audit (use-after-free, tran so, throttle, ghi dia that bai...).
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <thread>

#include "ApiContext.h"

using namespace shoppro;
namespace fs = std::filesystem;

namespace {

int gChecks = 0;
int gFailures = 0;

#define CHECK(cond)                                                                       \
    do {                                                                                  \
        ++gChecks;                                                                        \
        if (!(cond)) {                                                                    \
            ++gFailures;                                                                  \
            std::cerr << "THAT BAI " << __FILE__ << ":" << __LINE__ << "  " #cond << "\n"; \
        }                                                                                 \
    } while (0)

#define CHECK_STATUS(res_expr, expected)                                                                     \
    do {                                                                                                     \
        ++gChecks;                                                                                           \
        const Res r_ = (res_expr);                                                                           \
        if (r_.status != (expected)) {                                                                       \
            ++gFailures;                                                                                     \
            std::cerr << "THAT BAI " << __FILE__ << ":" << __LINE__ << "  status=" << r_.status             \
                      << " (mong doi " << (expected) << ") body=" << r_.body.dump().substr(0, 200) << "\n";  \
        }                                                                                                    \
    } while (0)

struct Res {
    int status = 0;
    Json body;
    std::map<std::string, std::string> headers;
};

struct Fixture {
    fs::path dir;
    fs::path dataFile;
    std::unique_ptr<JsonStore> store;
    std::unique_ptr<ActivityLog> log;
    std::unique_ptr<Ctx> ctx;
    Router router = buildRouter();

    Fixture() {
        dir = fs::temp_directory_path() / ("shoppro_test_" + crypto::randomHex(6));
        fs::create_directories(dir);
        dataFile = dir / "shoppro.json";
        boot();
    }
    ~Fixture() {
        std::error_code ignored;
        fs::remove_all(dir, ignored);
    }
    void boot() {   // (khoi dong lai) nap du lieu tu o dia
        const fs::path seed = SHOPPRO_TEST_SEED_FILE;
        store = std::make_unique<JsonStore>(dataFile, seed);
        const std::int64_t shift = store->seedShiftSeconds();
        Json data = store->load();
        log = std::make_unique<ActivityLog>(dir / "activity-log.jsonl", seed.parent_path() / "seed-activity.jsonl", shift);
        ctx = std::make_unique<Ctx>(*store, *log, std::move(data));
    }

    Res call(const std::string& method, const std::string& target, const Json& body = nullptr,
             const std::string& token = "", const std::string& ip = "10.0.0.1") {
        HttpRequest request;
        request.method = method;
        request.target = target;
        request.ip = ip;
        if (!token.empty()) request.headers["authorization"] = "Bearer " + token;
        if (!body.is_null()) request.body = body.dump();
        const HttpResponse response = router.dispatch(*ctx, request);
        return {response.status, response.body, response.headers};
    }
    std::string login(const std::string& email, const std::string& password, bool admin = false, const std::string& ip = "10.0.0.1") {
        const Res res = call("POST", admin ? "/api/admin/auth/login" : "/api/auth/login", {{"email", email}, {"password", password}}, "", ip);
        CHECK_STATUS(res, 200);
        return res.body.value("token", "");
    }
};

Json items(const Res& res) { return res.body.at("items"); }

// ------------------------------------------------------------------------------------
void testCatalog(Fixture& f) {
    Res res = f.call("GET", "/api/health");
    CHECK_STATUS(res, 200);
    res = f.call("GET", "/api/products?limit=100");
    CHECK_STATUS(res, 200);
    CHECK(res.body["total"].get<int>() >= 60);          // du lieu that, khong phai vai san pham
    for (const auto& p : items(res)) CHECK(!p.contains("visible"));   // co "visible" chi danh cho admin
    // tim kiem khong phan biet hoa/thuong va dau tieng Viet
    res = f.call("GET", "/api/products?search=tai%20nghe");
    CHECK_STATUS(res, 200);
    CHECK(res.body["total"].get<int>() >= 1);
    const int withAccent = res.body["total"].get<int>();
    res = f.call("GET", "/api/products?search=TAI%20NGHE");
    CHECK(res.body["total"].get<int>() == withAccent);
    res = f.call("GET", "/api/products?search=" + std::string("t%C3%A0i%20nghe"));   // "tài nghe" khong khop gi
    CHECK_STATUS(res, 200);
    res = f.call("GET", "/api/products?sort=price-asc&limit=5");
    CHECK(items(res)[0]["price"].get<std::int64_t>() <= items(res)[1]["price"].get<std::int64_t>());
    // hoi quy D: tran so khi phan trang
    res = f.call("GET", "/api/products?page=9223372036854775807&limit=100");
    CHECK_STATUS(res, 200);
    CHECK(items(res).empty());
    res = f.call("GET", "/api/products?page=abc");
    CHECK_STATUS(res, 400);
    res = f.call("GET", "/api/products?minPrice=99999999999999999999");
    CHECK_STATUS(res, 400);
    res = f.call("GET", "/api/products/999999");
    CHECK_STATUS(res, 404);
    res = f.call("GET", "/api/products/101");
    CHECK_STATUS(res, 200);
    CHECK(res.body["image"].get<std::string>() == "/img/products/101.svg");
    res = f.call("GET", "/api/products/165");   // san pham dang bi an
    CHECK_STATUS(res, 404);
    res = f.call("GET", "/api/products/101/related");
    CHECK_STATUS(res, 200);
    res = f.call("GET", "/api/categories");
    CHECK(res.body.size() == 8);
    res = f.call("POST", "/api/products", Json::object());
    CHECK_STATUS(res, 405);
    CHECK(res.headers.count("Allow") == 1);
    res = f.call("GET", "/api/khong-ton-tai");
    CHECK_STATUS(res, 404);
    res = f.call("GET", "/api/products/%ZZ");
    CHECK_STATUS(res, 400);
}

void testAuth(Fixture& f) {
    // dang ky: kiem tra du lieu dau vao
    Res res = f.call("POST", "/api/auth/register", {{"name", "A"}, {"email", "a@b.vn"}, {"phone", "0912345678"}, {"password", "Matkhau123"}}, "", "10.1.0.1");
    CHECK_STATUS(res, 400);   // ten qua ngan
    res = f.call("POST", "/api/auth/register", {{"name", "Test User"}, {"email", "khong-hop-le"}, {"phone", "0912345678"}, {"password", "Matkhau123"}}, "", "10.1.0.1");
    CHECK_STATUS(res, 400);
    res = f.call("POST", "/api/auth/register", {{"name", "Test User"}, {"email", "t@b.vn"}, {"phone", "abc"}, {"password", "Matkhau123"}}, "", "10.1.0.1");
    CHECK_STATUS(res, 400);
    res = f.call("POST", "/api/auth/register", {{"name", "Test User"}, {"email", "t@b.vn"}, {"phone", "0912345678"}, {"password", "12345678"}}, "", "10.1.0.1");
    CHECK_STATUS(res, 400);   // chi co so
    res = f.call("POST", "/api/auth/register", {{"name", "Test User"}, {"email", "Test.User@B.vn"}, {"phone", "0912345678"}, {"password", "Matkhau123"}}, "", "10.1.0.1");
    CHECK_STATUS(res, 201);
    CHECK(res.body["user"]["email"].get<std::string>() == "test.user@b.vn");   // chuan hoa chu thuong
    CHECK(res.body["token"].get<std::string>().size() == 64);
    res = f.call("POST", "/api/auth/register", {{"name", "Test User"}, {"email", "TEST.user@b.vn"}, {"phone", "0912345678"}, {"password", "Matkhau123"}}, "", "10.1.0.1");
    CHECK_STATUS(res, 409);   // trung email (khong phan biet hoa/thuong)

    // mat khau KHONG bao gio nam o dang ro / khong lo ra qua API / khong ghi vao nhat ky
    std::ifstream file(f.dataFile);
    const std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    CHECK(raw.find("Matkhau123") == std::string::npos);
    CHECK(raw.find("Demo@12345") == std::string::npos);
    CHECK(raw.find("Admin@12345") == std::string::npos);
    CHECK(raw.find("passwordHash") != std::string::npos);

    const std::string token = f.login("test.user@b.vn", "Matkhau123", false, "10.1.0.2");
    res = f.call("GET", "/api/auth/me", nullptr, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body.dump().find("password") == std::string::npos);

    // sai mat khau: thong bao chung, khong lo email co ton tai hay khong
    Res bad = f.call("POST", "/api/auth/login", {{"email", "test.user@b.vn"}, {"password", "sai-mat-khau"}}, "", "10.1.0.3");
    Res unknown = f.call("POST", "/api/auth/login", {{"email", "khong.co@b.vn"}, {"password", "sai-mat-khau"}}, "", "10.1.0.3");
    CHECK_STATUS(bad, 401);
    CHECK_STATUS(unknown, 401);
    CHECK(bad.body["error"] == unknown.body["error"]);

    // throttle: 5 lan sai lien tiep -> 429 + Retry-After; dang nhap dung cung bi chan trong thoi gian khoa
    for (int i = 0; i < 4; ++i) f.call("POST", "/api/auth/login", {{"email", "test.user@b.vn"}, {"password", "sai"}}, "", "10.1.0.9");
    Res blocked = f.call("POST", "/api/auth/login", {{"email", "test.user@b.vn"}, {"password", "Matkhau123"}}, "", "10.1.0.9");
    CHECK_STATUS(blocked, 200);   // moi 4 lan sai, chua qua nguong 5 -> dang nhap dung duoc
    for (int i = 0; i < 5; ++i) f.call("POST", "/api/auth/login", {{"email", "test.user@b.vn"}, {"password", "sai"}}, "", "10.1.0.10");
    blocked = f.call("POST", "/api/auth/login", {{"email", "test.user@b.vn"}, {"password", "Matkhau123"}}, "", "10.1.0.10");
    CHECK_STATUS(blocked, 429);
    CHECK(blocked.headers.count("Retry-After") == 1);

    // phan quyen
    const std::string admin = f.login("admin@shoppro.vn", "Admin@12345", true, "10.1.0.4");
    res = f.call("GET", "/api/admin/products", nullptr, token);
    CHECK_STATUS(res, 403);                                  // khach goi route admin
    res = f.call("GET", "/api/cart", nullptr, admin);
    CHECK_STATUS(res, 403);                                  // admin goi route khach
    res = f.call("GET", "/api/admin/products");
    CHECK_STATUS(res, 401);
    res = f.call("GET", "/api/cart", nullptr, "token-gia");
    CHECK_STATUS(res, 401);
    res = f.call("POST", "/api/admin/auth/login", {{"email", "khach@shoppro.vn"}, {"password", "Demo@12345"}}, "", "10.1.0.5");
    CHECK_STATUS(res, 401);                                  // tai khoan khach khong dang nhap duoc cong admin
    // dang xuat
    res = f.call("POST", "/api/auth/logout", Json::object(), token);
    CHECK_STATUS(res, 200);
    res = f.call("GET", "/api/auth/me", nullptr, token);
    CHECK_STATUS(res, 401);

    // ho so
    const std::string t2 = f.login("khach@shoppro.vn", "Demo@12345", false, "10.1.0.6");
    res = f.call("PATCH", "/api/users/me", {{"phone", "xx"}}, t2);
    CHECK_STATUS(res, 400);
    res = f.call("PATCH", "/api/users/me", {{"name", "Khách Demo 2"}, {"address", "1 Đường Thử Nghiệm, Quận 1"}}, t2);
    CHECK_STATUS(res, 200);
    CHECK(res.body["name"].get<std::string>() == "Khách Demo 2");
    res = f.call("PATCH", "/api/users/me", Json::object(), t2);
    CHECK_STATUS(res, 400);

    // du lieu hong
    HttpRequest broken;
    broken.method = "POST";
    broken.target = "/api/auth/login";
    broken.body = "{khong phai json";
    CHECK(f.router.dispatch(*f.ctx, broken).status == 400);
    broken.body = "[1,2]";
    CHECK(f.router.dispatch(*f.ctx, broken).status == 400);
    broken.body = std::string("{\"email\":\"a\xff\xfe\",\"password\":\"x\"}");   // UTF-8 sai
    CHECK(f.router.dispatch(*f.ctx, broken).status == 400);
}

Json cartLines(const Res& res) {
    Json out = Json::array();
    for (const auto& line : res.body.at("items")) out.push_back({line["product"]["id"], line["quantity"]});
    return out;
}

void testCartDsa(Fixture& f) {
    const std::string token = f.login("minhanh@example.com", "Demo@12345", false, "10.2.0.1");
    Res res = f.call("GET", "/api/cart", nullptr, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body["items"].empty());
    CHECK(res.body["dsa"]["linksValid"].get<bool>());

    // them: san pham moi + cong don
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", 2}}, token), 200);   // chuot 349.000
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 109}, {"quantity", 1}}, token), 200);   // ao thun 199.000
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 101}, {"quantity", 1}}, token), 200);   // tai nghe 1.890.000
    res = f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", 3}}, token);               // cong don -> 5
    CHECK_STATUS(res, 200);
    CHECK(res.body["distinctCount"].get<int>() == 3);
    CHECK(res.body["totalQuantity"].get<int>() == 5 + 1 + 1);
    CHECK(res.body["total"].get<std::int64_t>() == 5 * 349000 + 199000 + 1890000);
    CHECK(res.body["dsa"]["head"].get<int>() == 105);
    CHECK(res.body["dsa"]["tail"].get<int>() == 101);

    // validation
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", 0}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", -3}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", 1.5}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", "2"}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", 1e30}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 424242}, {"quantity", 1}}, token), 404);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 165}, {"quantity", 1}}, token), 404);   // dang bi an
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"quantity", 1}}, token), 404);                        // thieu productId
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 116}, {"quantity", 1}}, token), 409);   // het hang
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 106}, {"quantity", 8}}, token), 409);   // ton kho chi con 7
    CHECK_STATUS(f.call("PATCH", "/api/cart/items/105", {{"quantity", 100000}}, token), 409);
    CHECK_STATUS(f.call("PATCH", "/api/cart/items/999", {{"quantity", 1}}, token), 404);
    CHECK_STATUS(f.call("DELETE", "/api/cart/items/999", nullptr, token), 404);
    CHECK_STATUS(f.call("DELETE", "/api/cart/items/abc", nullptr, token), 404);

    // sap xep bang CartProcessing::ProductSort (bubble sort doi du lieu node)
    res = f.call("POST", "/api/cart/sort", {{"by", "price-asc"}}, token);
    CHECK_STATUS(res, 200);
    CHECK(cartLines(res) == Json::parse("[[109,1],[105,5],[101,1]]"));
    res = f.call("POST", "/api/cart/sort", {{"by", "price-desc"}}, token);
    CHECK(cartLines(res) == Json::parse("[[101,1],[105,5],[109,1]]"));
    res = f.call("POST", "/api/cart/sort", {{"by", "id"}}, token);
    CHECK(cartLines(res) == Json::parse("[[101,1],[105,5],[109,1]]"));
    res = f.call("POST", "/api/cart/sort", {{"by", "name"}}, token);   // "Ao thun" < "Chuot" < "Tai nghe" (khong dau)
    CHECK(cartLines(res) == Json::parse("[[109,1],[105,5],[101,1]]"));
    CHECK(res.body["items"][0]["product"]["name"].get<std::string>().rfind("Áo", 0) == 0);   // ten goc con nguyen dau
    CHECK_STATUS(f.call("POST", "/api/cart/sort", {{"by", "bay-ba"}}, token), 400);

    // tim kiem bang ProductSearch (khong phan biet dau/hoa thuong)
    res = f.call("GET", "/api/cart/search?q=CHUOT", nullptr, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body["total"].get<int>() == 1);
    res = f.call("GET", "/api/cart/search?q=" + std::string("%C3%A1o"), nullptr, token);
    CHECK(res.body["total"].get<int>() == 1);
    res = f.call("GET", "/api/cart/search?q=zzzz", nullptr, token);
    CHECK(res.body["total"].get<int>() == 0);
    CHECK_STATUS(f.call("GET", "/api/cart/search?q=", nullptr, token), 400);

    // cau truc danh sach lien ket doi
    res = f.call("GET", "/api/cart/structure?direction=forward", nullptr, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body["valid"].get<bool>());
    CHECK(res.body["size"].get<int>() == 3);
    const Json nodes = res.body["nodes"];
    CHECK(nodes[0]["prev"].is_null());
    CHECK(nodes[0]["next"] == nodes[1]["id"]);
    CHECK(nodes[1]["prev"] == nodes[0]["id"]);
    CHECK(nodes[2]["next"].is_null());
    res = f.call("GET", "/api/cart/structure?direction=backward", nullptr, token);
    CHECK(res.body["nodes"][0]["id"] == nodes[2]["id"]);
    CHECK_STATUS(f.call("GET", "/api/cart/structure?direction=sideways", nullptr, token), 400);

    // sua / xoa
    res = f.call("PATCH", "/api/cart/items/105", {{"quantity", 2}}, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body["totalQuantity"].get<int>() == 2 + 1 + 1);
    res = f.call("DELETE", "/api/cart/items/109", nullptr, token);
    CHECK(res.body["distinctCount"].get<int>() == 2);
    CHECK(res.body["dsa"]["linksValid"].get<bool>());

    // gio hang ben vung: khoi dong lai may chu van con dung thu tu & so luong
    const Json before = cartLines(f.call("GET", "/api/cart", nullptr, token));
    f.boot();
    const std::string again = f.login("minhanh@example.com", "Demo@12345", false, "10.2.0.2");
    CHECK(cartLines(f.call("GET", "/api/cart", nullptr, again)) == before);

    // moi nguoi dung co gio rieng
    const std::string other = f.login("quocbao@example.com", "Demo@12345", false, "10.2.0.3");
    CHECK(f.call("GET", "/api/cart", nullptr, other).body["items"].empty());

    // xoa ca gio
    res = f.call("DELETE", "/api/cart", nullptr, again);
    CHECK_STATUS(res, 200);
    CHECK(res.body["items"].empty());
    CHECK(res.body["dsa"]["head"].is_null());
}

void testOrdersAndStock(Fixture& f) {
    const std::string token = f.login("ngoclan@example.com", "Demo@12345", false, "10.3.0.1");
    const std::string admin = f.login("admin@shoppro.vn", "Admin@12345", true, "10.3.0.2");
    const auto stockOf = [&](int id) { return f.call("GET", "/api/products/" + std::to_string(id)).body["stock"].get<int>(); };

    CHECK_STATUS(f.call("POST", "/api/orders", {{"phone", "0912345678"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"}}, token), 409);   // gio trong
    const int stockBefore = stockOf(104);
    f.call("POST", "/api/cart/items", {{"productId", 104}, {"quantity", 2}}, token);
    f.call("POST", "/api/cart/items", {{"productId", 108}, {"quantity", 3}}, token);

    // du lieu giao hang khong hop le
    CHECK_STATUS(f.call("POST", "/api/orders", {{"phone", "abc"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/orders", {{"phone", "0912345678"}, {"shippingAddress", "ngan"}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/orders", {{"phone", "0912345678"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"}, {"paymentMethod", "bitcoin"}}, token), 400);
    // danh sach gui len khong khop gio hang tren may chu
    CHECK_STATUS(f.call("POST", "/api/orders", {{"phone", "0912345678"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"},
                                                 {"items", Json::array({{{"productId", 104}, {"quantity", 9}}})}}, token), 409);

    // admin ha ton kho xuong thap hon so luong trong gio -> dat hang bi chan, ton kho/gio khong doi
    CHECK_STATUS(f.call("PATCH", "/api/admin/products/104/stock", {{"stock", 1}}, admin), 200);
    Res res = f.call("POST", "/api/orders", {{"phone", "0912345678"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"}}, token);
    CHECK_STATUS(res, 409);
    CHECK(stockOf(104) == 1);
    CHECK(f.call("GET", "/api/cart", nullptr, token).body["items"].size() == 2);
    CHECK_STATUS(f.call("PATCH", "/api/admin/products/104/stock", {{"stock", stockBefore}}, admin), 200);

    // dat hang thanh cong: tru kho, xoa gio, tong tien tinh o server
    const int stock108 = stockOf(108);
    res = f.call("POST", "/api/orders", {{"customerName", "Võ Ngọc Lan"}, {"phone", "0912345678"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"},
                                        {"note", "<script>alert(1)</script>"}, {"paymentMethod", "bank"}}, token);
    CHECK_STATUS(res, 201);
    const std::string orderId = res.body["id"];
    CHECK(orderId.rfind("SP-", 0) == 0);
    CHECK(res.body["status"] == "pending");
    CHECK(res.body["note"] == "<script>alert(1)</script>");   // luu nguyen van (Frontend moi escape khi hien thi)
    std::int64_t expected = 0;
    for (const auto& line : res.body["items"]) expected += line["product"]["price"].get<std::int64_t>() * line["quantity"].get<std::int64_t>();
    CHECK(res.body["total"].get<std::int64_t>() == expected);
    CHECK(stockOf(104) == stockBefore - 2);
    CHECK(stockOf(108) == stock108 - 3);
    CHECK(f.call("GET", "/api/cart", nullptr, token).body["items"].empty());

    // chi chu don moi thay don
    const std::string other = f.login("quocbao@example.com", "Demo@12345", false, "10.3.0.3");
    CHECK_STATUS(f.call("GET", "/api/orders/" + orderId, nullptr, other), 404);
    CHECK_STATUS(f.call("POST", "/api/orders/" + orderId + "/cancel", Json::object(), other), 404);
    CHECK_STATUS(f.call("GET", "/api/orders/" + orderId, nullptr, token), 200);

    // may trang thai cua admin
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "completed"}}, admin), 409);   // khong nhay coc
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "banana"}}, admin), 400);
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/SP-000000-0000/status", {{"status", "confirmed"}}, admin), 404);
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "confirmed"}}, admin), 200);
    CHECK_STATUS(f.call("POST", "/api/orders/" + orderId + "/cancel", Json::object(), token), 409);                   // khach khong huy don da xac nhan
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "shipping"}}, admin), 200);
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "cancelled"}}, admin), 409); // dang giao khong huy duoc
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "completed"}}, admin), 200);
    CHECK_STATUS(f.call("PATCH", "/api/admin/orders/" + orderId + "/status", {{"status", "pending"}}, admin), 409);   // trang thai cuoi

    // khach tu huy don dang cho -> tra kho
    f.call("POST", "/api/cart/items", {{"productId", 104}, {"quantity", 4}}, token);
    res = f.call("POST", "/api/orders", {{"phone", "0912345678"}, {"shippingAddress", "12 Nguyen Hue, Quan 1"}}, token);
    CHECK_STATUS(res, 201);
    const std::string second = res.body["id"];
    CHECK(stockOf(104) == stockBefore - 2 - 4);
    CHECK_STATUS(f.call("POST", "/api/orders/" + second + "/cancel", Json::object(), token), 200);
    CHECK(stockOf(104) == stockBefore - 2);                           // khong tra kho hai lan
    CHECK_STATUS(f.call("POST", "/api/orders/" + second + "/cancel", Json::object(), token), 409);
    CHECK(stockOf(104) == stockBefore - 2);

    // nhat ky ghi nhan, khong chua bi mat
    res = f.call("GET", "/api/admin/activity-logs?action=order.placed&limit=5", nullptr, admin);
    CHECK_STATUS(res, 200);
    CHECK(res.body["total"].get<int>() >= 3);
    CHECK(res.body["items"][0]["action"] == "order.placed");
    CHECK(f.call("GET", "/api/admin/activity-logs?from=2026-13-45", nullptr, admin).status == 400);
    res = f.call("GET", "/api/admin/activity-logs?limit=200", nullptr, admin);
    CHECK(res.body.dump().find("Demo@12345") == std::string::npos);
    CHECK(res.body.dump().find("passwordHash") == std::string::npos);
}

void testAdminCatalog(Fixture& f) {
    const std::string admin = f.login("admin@shoppro.vn", "Admin@12345", true, "10.4.0.1");
    const std::string user = f.login("thuha@example.com", "Demo@12345", false, "10.4.0.2");
    const Json good = {{"name", "Sản phẩm thử nghiệm"}, {"categoryId", 1}, {"price", 150000}, {"stock", 20},
                       {"description", "Mô tả"}, {"image", "/img/products/101.svg"}, {"visible", true}};
    const auto with = [&](const char* key, Json value) { Json copy = good; copy[key] = std::move(value); return copy; };

    CHECK_STATUS(f.call("POST", "/api/admin/products", with("price", -1), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("price", 10000000001LL), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("price", 1.5), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("stock", 100001), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("categoryId", 99), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("name", ""), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("image", "javascript:alert(1)"), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("image", "//evil.example/x.png"), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", with("image", "/img/../secret.svg"), admin), 400);
    CHECK_STATUS(f.call("POST", "/api/admin/products", good, user), 403);

    Res res = f.call("POST", "/api/admin/products", good, admin);
    CHECK_STATUS(res, 201);
    const int id = res.body["id"];
    CHECK(id > 165);
    CHECK(f.call("GET", "/api/products/" + std::to_string(id)).status == 200);

    // gia thay doi -> gio hang cua khach cap nhat (ProductManager::capNhatThongTin)
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", id}, {"quantity", 2}}, user), 200);
    res = f.call("PUT", "/api/admin/products/" + std::to_string(id), with("price", 200000), admin);
    CHECK_STATUS(res, 200);
    res = f.call("GET", "/api/cart", nullptr, user);
    CHECK(res.body["total"].get<std::int64_t>() == 400000);
    // an san pham -> bien mat khoi cua hang va gio hang
    CHECK_STATUS(f.call("PATCH", "/api/admin/products/" + std::to_string(id) + "/visibility", {{"visible", false}}, admin), 200);
    CHECK(f.call("GET", "/api/products/" + std::to_string(id)).status == 404);
    CHECK(f.call("GET", "/api/cart", nullptr, user).body["items"].empty());
    CHECK_STATUS(f.call("PATCH", "/api/admin/products/" + std::to_string(id) + "/visibility", {{"visible", "yes"}}, admin), 400);
    // xoa
    CHECK_STATUS(f.call("DELETE", "/api/admin/products/" + std::to_string(id), nullptr, admin), 200);
    CHECK_STATUS(f.call("DELETE", "/api/admin/products/" + std::to_string(id), nullptr, admin), 404);

    // danh muc
    CHECK_STATUS(f.call("POST", "/api/admin/categories", {{"name", "Điện tử & phụ kiện"}}, admin), 409);   // trung (bo dau, hoa/thuong)
    res = f.call("POST", "/api/admin/categories", {{"name", "Đồ chơi"}, {"description", "Đồ chơi trẻ em"}}, admin);
    CHECK_STATUS(res, 201);
    const int catId = res.body["id"];
    CHECK_STATUS(f.call("PUT", "/api/admin/categories/" + std::to_string(catId), {{"name", "Đồ chơi & Game"}}, admin), 200);
    CHECK_STATUS(f.call("DELETE", "/api/admin/categories/1", nullptr, admin), 409);    // con san pham
    CHECK_STATUS(f.call("DELETE", "/api/admin/categories/" + std::to_string(catId), nullptr, admin), 200);

    // cai dat
    const Json settings = {{"storeName", "ShopPro"}, {"supportEmail", "a@b.vn"}, {"supportPhone", "1900 1234"}, {"lowStockThreshold", 5}, {"currency", "VND"}};
    CHECK_STATUS(f.call("PUT", "/api/admin/settings", settings, admin), 200);
    Json badSettings = settings;
    badSettings["supportEmail"] = "khong-hop-le";
    CHECK_STATUS(f.call("PUT", "/api/admin/settings", badSettings, admin), 400);
    badSettings = settings;
    badSettings["currency"] = "EUR";
    CHECK_STATUS(f.call("PUT", "/api/admin/settings", badSettings, admin), 400);
    badSettings = settings;
    badSettings["lowStockThreshold"] = -4;
    CHECK_STATUS(f.call("PUT", "/api/admin/settings", badSettings, admin), 400);

    // khoa khach -> khong dang nhap duoc va phien cu bi thu hoi
    CHECK_STATUS(f.call("GET", "/api/cart", nullptr, user), 200);
    CHECK_STATUS(f.call("PATCH", "/api/admin/customers/cus-1004/status", {{"status", "locked"}}, admin), 200);
    CHECK_STATUS(f.call("GET", "/api/cart", nullptr, user), 401);
    Res locked = f.call("POST", "/api/auth/login", {{"email", "thuha@example.com"}, {"password", "Demo@12345"}}, "", "10.4.0.3");
    CHECK_STATUS(locked, 403);
    CHECK_STATUS(f.call("PATCH", "/api/admin/customers/cus-1004/status", {{"status", "active"}}, admin), 200);
    CHECK_STATUS(f.call("PATCH", "/api/admin/customers/adm-0001/status", {{"status", "locked"}}, admin), 404);   // khong khoa duoc admin
    CHECK_STATUS(f.call("PATCH", "/api/admin/customers/cus-1004/status", {{"status", "banana"}}, admin), 400);
}

void testReportsAndDashboard(Fixture& f) {
    const std::string admin = f.login("admin@shoppro.vn", "Admin@12345", true, "10.5.0.1");
    Res res = f.call("GET", "/api/admin/dashboard", nullptr, admin);
    CHECK_STATUS(res, 200);
    CHECK(res.body["productCount"].get<int>() >= 60);
    CHECK(res.body["revenueSeries"].size() == 14);
    CHECK(res.body["bestSellers"].size() == 5);
    CHECK(res.body["customerCount"].get<int>() >= 15);
    CHECK(res.body["revenue"].get<std::int64_t>() > 0);

    // du lieu mau duoc dich ve "hien tai": don moi nhat nam trong vai gio gan day
    res = f.call("GET", "/api/admin/orders", nullptr, admin);
    CHECK(res.body.size() >= 60);
    const std::int64_t newest = timeutil::parseIso(res.body[0]["createdAt"].get<std::string>());
    CHECK(timeutil::epochSeconds() - newest < 3 * 86400);
    CHECK(newest <= timeutil::epochSeconds() + 3600);

    res = f.call("GET", "/api/admin/reports/summary?from=2020-01-01&to=2020-01-10", nullptr, admin);
    CHECK_STATUS(res, 200);
    CHECK(res.body["orderCount"].get<int>() == 0);
    CHECK(res.body["revenueSeries"].size() == 10);
    CHECK_STATUS(f.call("GET", "/api/admin/reports/summary?from=2026-02-30&to=2026-03-01", nullptr, admin), 400);
    CHECK_STATUS(f.call("GET", "/api/admin/reports/summary?from=2026-05-01&to=2026-04-01", nullptr, admin), 400);
    CHECK_STATUS(f.call("GET", "/api/admin/reports/summary?from=2000-01-01&to=2026-04-01", nullptr, admin), 400);   // > 366 ngay
    res = f.call("GET", "/api/admin/reports/summary", nullptr, admin);
    CHECK_STATUS(res, 200);
    CHECK(res.body["categorySeries"].size() >= 1);
    CHECK(res.body["topProducts"].size() >= 1);

    res = f.call("GET", "/api/admin/orders?status=pending", nullptr, admin);
    for (const auto& o : res.body) CHECK(o["status"] == "pending");
    res = f.call("GET", "/api/admin/customers", nullptr, admin);
    CHECK(res.body.size() >= 15);   // 15 khach mau (+ tai khoan do cac kiem thu khac vua dang ky)
    CHECK(res.body.dump().find("passwordHash") == std::string::npos);
    CHECK_STATUS(f.call("GET", "/api/admin/customers/cus-1002/orders", nullptr, admin), 200);
    CHECK_STATUS(f.call("GET", "/api/admin/customers/xyz/orders", nullptr, admin), 404);
}

// Ghi dia that bai: API tra 500, du lieu va gio hang trong RAM KHONG bi thay doi
void testPersistFailure(Fixture& f) {
    const std::string token = f.login("hoangnam@example.com", "Demo@12345", false, "10.6.0.1");
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 105}, {"quantity", 1}}, token), 200);

    fs::path tmp = f.dataFile;
    tmp += ".tmp";
    fs::create_directories(tmp);   // thu muc chiem cho tep tam -> persist() chac chan that bai

    Res res = f.call("POST", "/api/cart/items", {{"productId", 109}, {"quantity", 1}}, token);
    CHECK_STATUS(res, 500);
    res = f.call("GET", "/api/cart", nullptr, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body["items"].size() == 1);                       // gio van nhu cu, khong lech so voi dia
    CHECK(res.body["dsa"]["linksValid"].get<bool>());
    res = f.call("DELETE", "/api/cart", nullptr, token);
    CHECK_STATUS(res, 500);
    CHECK(f.call("GET", "/api/cart", nullptr, token).body["items"].size() == 1);   // xoa gio that bai -> hoan tac RAM

    fs::remove_all(tmp);
    CHECK_STATUS(f.call("POST", "/api/cart/items", {{"productId", 109}, {"quantity", 1}}, token), 200);   // hoi phuc
    CHECK(f.call("GET", "/api/cart", nullptr, token).body["items"].size() == 2);
}

// Nhieu luong cung goi API: khong crash, khong mat du lieu (chay duoi ASan/UBSan)
void testConcurrency(Fixture& f) {
    std::vector<std::string> tokens;
    const char* emails[] = {"thanhtung@example.com", "maiphuong@example.com", "giahuy@example.com", "khanhlinh@example.com"};
    for (int i = 0; i < 4; ++i) tokens.push_back(f.login(emails[i], "Demo@12345", false, "10.7.0." + std::to_string(i + 1)));
    std::atomic<int> failures{0};
    std::vector<std::thread> workers;
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back([&, t] {
            for (int round = 0; round < 15; ++round) {
                const int product = 101 + (round % 6);
                if (f.call("POST", "/api/cart/items", {{"productId", product}, {"quantity", 1}}, tokens[t]).status != 200) ++failures;
                if (round % 3 == 0 && f.call("POST", "/api/cart/sort", {{"by", "price-desc"}}, tokens[t]).status != 200) ++failures;
                if (f.call("GET", "/api/cart/search?q=a", nullptr, tokens[t]).status != 200) ++failures;
                if (round % 4 == 0 && f.call("DELETE", "/api/cart/items/" + std::to_string(product), nullptr, tokens[t]).status != 200) ++failures;
                f.call("GET", "/api/products?limit=20");
            }
        });
    }
    for (auto& w : workers) w.join();
    CHECK(failures.load() == 0);
    for (const auto& token : tokens) {
        const Res res = f.call("GET", "/api/cart", nullptr, token);
        CHECK(res.body["dsa"]["linksValid"].get<bool>());
    }
}

void testAssistant(Fixture& f) {
    const std::string token = f.login("tuankiet@example.com", "Demo@12345", false, "10.8.0.1");
    Res res = f.call("POST", "/api/ai/chat", {{"message", "gợi ý tai nghe dưới 2 triệu"}}, token);
    CHECK_STATUS(res, 200);
    CHECK(res.body["products"].size() >= 1);
    for (const auto& p : res.body["products"]) CHECK(p["price"].get<std::int64_t>() <= 2000000);
    CHECK(res.body["filters"]["maxPrice"].get<int>() == 2000000);
    // hoi quy: so cuc lon khong lam may chu ngoai le (truoc day dung stod)
    res = f.call("POST", "/api/ai/chat", {{"message", "toi can mua hang " + std::string(400, '9') + " trieu"}}, token);
    CHECK_STATUS(res, 200);
    res = f.call("POST", "/api/ai/chat", {{"message", "so sánh bình giữ nhiệt 500k"}}, token);
    CHECK(res.body["comparison"].get<bool>());
    CHECK(res.body["products"].size() <= 2);
    CHECK_STATUS(f.call("POST", "/api/ai/chat", {{"message", ""}}, token), 400);
    CHECK_STATUS(f.call("POST", "/api/ai/chat", {{"message", "xin chao"}}), 401);
}

}  // namespace

int main() {
    Fixture f;
    testCatalog(f);
    testAuth(f);
    testCartDsa(f);
    testOrdersAndStock(f);
    testAdminCatalog(f);
    testReportsAndDashboard(f);
    testPersistFailure(f);
    testConcurrency(f);
    testAssistant(f);
    std::cout << "test_api: " << (gChecks - gFailures) << "/" << gChecks << " kiem tra dat\n";
    return gFailures == 0 ? 0 : 1;
}
