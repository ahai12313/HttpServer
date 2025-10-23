#include "HttpServer.h"
#include "Request.h"
#include "Response.h"
#include <algorithm>
#include <asm-generic/socket.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <ostream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "JsonValue.h"
#include "split.h"

class HttpServer::Impl {
  public:
    Impl(const Config &config) : port_(config.port), server_fd_(-1) {
        root_dir_ = fs::absolute(config.root_dir);
        if (!fs::is_directory(root_dir_)) {
            throw std::runtime_error("Path is not a directory: " + root_dir_);
        }
    }
    ~Impl() { close(server_fd_); }

    void start();
    void run();

    // 设置请求处理器
    void set_request_handler(RequestHandler handler) {
        request_handler_ = std::move(handler);
    }

    // 设置错误处理器
    void set_error_handler(ErrorHandler handler) {
        error_handler_ = std::move(handler);
    }
    void handle_client(int client_fd);
    Request parse_request(int client_fd, const std::string &request);

    void set_root_directory(const std::string &path);

    const std::string &get_root_path();

  private:
    int port_;
    int server_fd_;
    std::string root_dir_;

    RequestHandler request_handler_;
    ErrorHandler error_handler_;
};

HttpServer::HttpServer(const Config &config)
    : impl_(std::make_unique<Impl>(config)) {}

HttpServer::~HttpServer() {}

void HttpServer::Impl::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("socket creation failed");
        return;
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        perror("listen failed");
        return;
    }
    SPDLOG_INFO("HTTP server running on port {}", port_);
    return;
}

void HttpServer::start() { impl_->start(); }

void HttpServer::Impl::run() {
    while (true) {

        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd =
            accept(server_fd_, (sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            int err = errno;
            switch (err) {
            case EBADF:
                std::cerr << "accept: server_fd_ is not a valid file descriptor"
                          << std::endl;
                break;
            case ENOTSOCK:
                std::cerr << "accept: server_fd_ is not a socket" << std::endl;
                break;
            case EOPNOTSUPP:
                std::cerr << "accept: socket is not of type SOCK_STREAM"
                          << std::endl;
                break;
            case EINVAL:
                std::cerr << "accept: socket is not listening for connections"
                          << std::endl;
                break;
            default:
                perror("accept failed");
            }
            if (err == EBADF || err == ENOTSOCK) {
                exit(EXIT_FAILURE);
            }
            continue;
        }

        std::thread([this, client_fd] {
            handle_client(client_fd);
            close(client_fd);
        }).detach();
    }
}

void HttpServer::run() { impl_->run(); }

void HttpServer::set_request_handler(RequestHandler handler) {
    impl_->set_request_handler(std::move(handler));
}

void HttpServer::set_error_handler(ErrorHandler handler) {
    impl_->set_error_handler(std::move(handler));
}

Request HttpServer::Impl::parse_request(int client_fd,
                                        const std::string &request) {

    std::istringstream stream(request);
    Request req(client_fd);

    stream >> req.method >> req.path >> req.version;

    req.splited_path = split_path(req.path, '/');

    std::transform(req.method.begin(), req.method.end(), req.method.begin(),
                   ::toupper);

    stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string header_line;
    while (std::getline(stream, header_line) && header_line != "\r") {
        if (header_line.back() == '\r')
            header_line.pop_back();

        auto colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = header_line.substr(0, colon_pos);
            std::string value = header_line.substr(colon_pos + 1);

            key.erase(0, key.find_first_not_of(" "));
            key.erase(key.find_last_not_of(" ") + 1);
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);

            req.headers[key] = value;
        }
    }

    // 解析请求体（如果有）
    if (req.headers.find("Content-Length") != req.headers.end()) {
        size_t content_length = std::stoul(req.headers["Content-Length"]);
        req.body.resize(content_length);
        stream.read(reinterpret_cast<char*>(req.body.data()), content_length);
        if (req.isBodyLikelyString()) {
            req.body_str = std::string(req.body.begin(), req.body.end());
        }
        req.json_value = JsonValue::parse(req.body_str);
    }

    return req;
}

const std::string &HttpServer::get_root_path() {
    return impl_->get_root_path();
}

const std::string &HttpServer::Impl::get_root_path() { return root_dir_; }

void HttpServer::Impl::handle_client(int client_fd) {

    try {
        char buffer[4096] = {0};
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            perror("recv failed");
            return;
        }

        Request request = parse_request(client_fd, buffer);
        Response response(client_fd);
        request_handler_(request, response);

    } catch (const std::exception& e) {
        Request error_req(client_fd);
        Response error_res(client_fd);
        error_handler_(error_req, error_res, e);
    }
}

// 添加静态文件服务
void HttpServer::set_root_directory(const std::string &path) {
    impl_->set_root_directory(path);
}

void HttpServer::Impl::set_root_directory(const std::string &path) {
    root_dir_ = path;
}