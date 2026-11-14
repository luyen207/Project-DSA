// May chu HTTP/1.1 toi gian (socket thuan, khong thu vien ngoai): moi ket noi mot luong rieng,
// keep-alive, gioi han kich thuoc/thoi gian, phuc vu tep tinh cho Frontend + chuyen /api/* cho REST API.
#pragma once

#include <atomic>
#include <filesystem>
#include <functional>
#include <string>

#include "HttpTypes.h"

namespace shoppro {

struct ServerConfig {
    std::string host = "127.0.0.1";       // 0.0.0.0 de mo ra mang LAN / container
    unsigned short port = 8080;
    std::filesystem::path webRoot;        // thu muc Frontend (rong = chi phuc vu API)
    bool trustProxy = false;              // tin X-Forwarded-For (chi bat khi dung sau reverse proxy)
    std::size_t maxConnections = 256;
    bool logRequests = true;
};

using ApiHandler = std::function<HttpResponse(const HttpRequest&)>;

// Chay cho den khi stopFlag = true. Tra ve 0 neu dung binh thuong, khac 0 neu khong mo duoc cong.
int runHttpServer(const ServerConfig& config, const ApiHandler& api, const std::atomic<bool>& stopFlag);

}  // namespace shoppro
