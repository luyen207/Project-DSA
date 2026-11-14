#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace shoppro {

struct HttpRequest {
    std::string method;
    std::string target;                          // duong dan + query string
    std::map<std::string, std::string> headers;  // khoa viet thuong
    std::string body;
    std::string ip;                              // dia chi client (hoac X-Forwarded-For khi tin cay proxy)
};

struct HttpResponse {
    int status = 200;
    nlohmann::json body;
    std::map<std::string, std::string> headers;  // header bo sung (vd Retry-After)
};

}  // namespace shoppro
