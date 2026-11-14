// Cau noi giua REST API va 4 module DSA cua nhom:
//   Common (DoublyLinkedList) + ProductManager + ProductSearch + CartProcessing,
// duoc dong goi san trong CartController. Moi khach hang co MOT CartController
// (danh sach lien ket doi) giu trong RAM; du lieu ben vung nam trong data["carts"].
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "CartController.h"
#include "Support.h"

namespace shoppro {

class CartEngine {
public:
    // Gio hang cua user (nap tu data["carts"][userId] o lan dung dau tien)
    CartController& cart(const std::string& userId, const Json& data);

    // Dong bo voi danh muc: bo san pham da bi xoa, cap nhat ten/gia moi (ProductManager::capNhatThongTin)
    void syncWithCatalog(const std::string& userId, const Json& data);

    // Dung lai tu JSON (dung khi ghi file that bai -> hoan tac thao tac)
    void rebuild(const std::string& userId, const Json& data);

    // Cac dong gio theo thu tu head -> tail: [{productId, quantity}] (de luu JSON)
    Json lines(const std::string& userId, const Json& data);

    // Sap xep tai cho bang CartProcessing::ProductSort. by: price-asc | price-desc | name | id
    // Tra ve false neu "by" khong hop le.
    bool sortBy(const std::string& userId, const Json& data, const std::string& by);

    // Tim theo ten (ProductSearch, khong phan biet hoa/thuong va dau tieng Viet). Tra ve productId.
    std::vector<std::int64_t> searchByName(const std::string& userId, const Json& data, const std::string& query);

    // Cau truc danh sach lien ket doi de hien thi/giai thich: head, tail, tung node (prev/next)
    Json structure(const std::string& userId, const Json& data, bool forward);

private:
    std::map<std::string, std::unique_ptr<CartController>> carts_;
};

}  // namespace shoppro
