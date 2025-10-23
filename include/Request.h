#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <vector>

#include "JsonValue.h"

namespace fs = std::filesystem;

// 请求类
class Request {
public:
    Request(int fd) : client_fd_(fd) {}
    Request() = default;

    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::vector<uint8_t> body;
    std::string body_str;
    fs::path full_path;
    std::vector<std::string> splited_path;
    std::vector<std::string> params;

    JsonValue json_value;

    std::unordered_map<std::string, std::string> query_params() const;
    std::string get_path_param(const std::string& key) const;
    std::string client_ip() const;

    bool isBodyLikelyString();

    int client_fd_;

    void info();

};

#endif