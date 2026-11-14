#include "CartEngine.h"

#include <sstream>

#include "CartCalculator.h"
#include "ProductSearch.h"

namespace shoppro {
namespace {

// Duyet an toan (gioi han soNode buoc) cac node theo chieu xuoi
template <typename Fn>
void forEachNode(const DanhSach& ds, Fn fn) {
    int steps = 0;
    for (const Node* p = ds.head; p != nullptr && steps < ds.soNode; p = p->next, ++steps) fn(*p);
}

}  // namespace

CartController& CartEngine::cart(const std::string& userId, const Json& data) {
    auto found = carts_.find(userId);
    if (found == carts_.end()) {
        carts_[userId] = std::make_unique<CartController>();
        rebuild(userId, data);
        found = carts_.find(userId);
    }
    return *found->second;
}

void CartEngine::rebuild(const std::string& userId, const Json& data) {
    auto& slot = carts_[userId];
    if (!slot) slot = std::make_unique<CartController>();
    CartController& controller = *slot;
    controller.lamTrongGio();
    if (!data.contains("carts") || !data["carts"].contains(userId) || !data["carts"][userId].is_array()) return;
    for (const auto& line : data["carts"][userId]) {
        const std::int64_t productId = jsonx::integer(line, "productId", -1);
        const std::int64_t quantity = jsonx::integer(line, "quantity", 0);
        const Json* product = jsonx::findById(data.at("products"), productId);
        if (!product || !product->value("visible", true) || productId <= 0 || productId > 2147483647LL ||
            quantity < 1 || quantity > SO_LUONG_TOI_DA) continue;
        controller.themSanPham(static_cast<int>(productId), product->value("name", ""),
                               static_cast<double>(jsonx::integer(*product, "price")), static_cast<int>(quantity));
    }
}

void CartEngine::syncWithCatalog(const std::string& userId, const Json& data) {
    CartController& controller = cart(userId, data);
    struct Change { int id; bool remove; std::string name; double price; };
    std::vector<Change> changes;
    forEachNode(controller.danhSach(), [&](const Node& node) {
        const Json* product = jsonx::findById(data.at("products"), node.id);
        if (!product || !product->value("visible", true)) {   // da xoa hoac dang bi an
            changes.push_back({node.id, true, "", 0});
            return;
        }
        const std::string name = product->value("name", "");
        const double price = static_cast<double>(jsonx::integer(*product, "price"));
        if (name != node.ten || price != node.gia) changes.push_back({node.id, false, name, price});
    });
    for (const auto& change : changes) {
        if (change.remove) controller.xoaSanPham(change.id);
        else controller.capNhatThongTin(change.id, change.name, change.price);   // ProductManager
    }
}

Json CartEngine::lines(const std::string& userId, const Json& data) {
    Json result = Json::array();
    forEachNode(cart(userId, data).danhSach(), [&](const Node& node) {
        result.push_back({{"productId", node.id}, {"quantity", node.soLuong}});
    });
    return result;
}

bool CartEngine::sortBy(const std::string& userId, const Json& data, const std::string& by) {
    CartController& controller = cart(userId, data);
    if (by == "price-asc") { controller.sapXep(CartController::GIA_TANG); return true; }
    if (by == "price-desc") { controller.sapXep(CartController::GIA_GIAM); return true; }
    if (by == "id") { controller.sapXep(CartController::THEO_ID); return true; }
    if (by != "name") return false;

    // ProductSort so sanh theo byte (A-Z). De tieng Viet co dau xep dung thu tu,
    // tam thoi dat ten = ten khong dau, sap xep roi tra ten goc ve.
    std::map<int, std::string> original;
    std::map<int, double> prices;
    forEachNode(controller.danhSach(), [&](const Node& node) {
        original[node.id] = node.ten;
        prices[node.id] = node.gia;
    });
    for (const auto& entry : original) {
        const std::string key = text::searchKey(entry.second);
        controller.capNhatThongTin(entry.first, key.empty() ? "-" : key, prices[entry.first]);
    }
    controller.sapXep(CartController::THEO_TEN);
    for (const auto& entry : original) controller.capNhatThongTin(entry.first, entry.second, prices[entry.first]);
    return true;
}

std::vector<std::int64_t> CartEngine::searchByName(const std::string& userId, const Json& data,
                                                    const std::string& query) {
    CartController& controller = cart(userId, data);
    // Danh sach tam voi ten khong dau -> ProductSearch::timTheoTen tim duoc ca tieng Viet
    DanhSach folded;
    forEachNode(controller.danhSach(), [&](const Node& node) {
        std::string key = text::searchKey(node.ten);
        if (key.empty()) key = "-";
        Node* copy = taoNode(node.id, key, node.gia, node.soLuong);
        if (copy != nullptr && !themCuoi(folded, copy)) delete copy;
    });
    std::vector<std::int64_t> ids;
    for (const Node* found : ProductSearch::timTheoTen(folded, text::searchKey(query))) ids.push_back(found->id);
    return ids;
}

Json CartEngine::structure(const std::string& userId, const Json& data, bool forward) {
    CartController& controller = cart(userId, data);
    const DanhSach& ds = controller.danhSach();
    Json nodes = Json::array();
    int steps = 0;
    for (const Node* p = forward ? ds.head : ds.tail; p != nullptr && steps < ds.soNode;
         p = forward ? p->next : p->prev, ++steps) {
        nodes.push_back({{"id", p->id},
                         {"name", p->ten},
                         {"quantity", p->soLuong},
                         {"price", static_cast<std::int64_t>(p->gia)},
                         {"prev", p->prev ? Json(p->prev->id) : Json(nullptr)},
                         {"next", p->next ? Json(p->next->id) : Json(nullptr)}});
    }
    std::ostringstream view;
    controller.hienThiTatCa(forward, view);   // ProductDisplay (bang chu giong CLI)
    return {{"direction", forward ? "forward" : "backward"},
            {"size", ds.soNode},
            {"head", ds.head ? Json(ds.head->id) : Json(nullptr)},
            {"tail", ds.tail ? Json(ds.tail->id) : Json(nullptr)},
            {"valid", controller.lienKetHopLe()},   // DoublyLinkedList::kiemTraLienKet
            {"nodes", nodes},
            {"textView", view.str()}};
}

}  // namespace shoppro
