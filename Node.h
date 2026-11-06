#ifndef NODE_H
#define NODE_H

#include <string>
using namespace std;
// Một dòng trong giỏ hàng
struct Node {
    int id;              // Mã sản phẩm
    string ten;          // Tên sản phẩm
    double gia;          // Giá sản phẩm
    int soLuong;         // Số lượng
    Node* prev;          // Nút đứng trước
    Node* next;          // Nút đứng sau
};

// Tạo một nút mới, trả về nullptr nếu dữ liệu không hợp lệ
Node* taoNode(int id, std::string ten, double gia, int soLuong);

#endif