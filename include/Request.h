#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <filesystem>
#include <unordered_map>
#include <spdlog/spdlog.h>

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
    std::string body;
    fs::path full_path;

    std::unordered_map<std::string, std::string> query_params() const;
    std::string get_path_param(const std::string& key) const;
    std::string client_ip() const;

    int client_fd_;

    void info();
};

#endif