#include "Response.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace fs = std::filesystem;

// Response类实现
Response::Response(int client_fd) : client_fd_(client_fd), status_code_(200) {
    // 设置默认头部
    header("Server", "HttpServer/1.0");
    headers_["Content-Type"] = "text/plain";
}

Response::~Response() {}

Response &Response::status(int code) {
    status_code_ = code;
    return *this;
}

Response &Response::header(const std::string &key, const std::string &value) {
    headers_[key] = value;
    return *this;
}

void Response::send_headers() {
    SPDLOG_DEBUG("send headers {}", status_code_);
    if (headers_sent_)
        return;
    std::ostringstream headers;
    headers << "HTTP/1.1 " << status_code_ << " ";
    switch (status_code_) {
    case 200:
        headers << "OK";
        break;
    case 404:
        headers << "Not Found";
        break;
    case 500:
        headers << "Internal Server Error";
        break;
    default:
        headers << "Unknown";
        break;
    }
    headers << "\r\n";

    for (const auto &[key, value] : headers_) {
        headers << key << ": " << value << "\r\n";
    }

    headers << "\r\n"; // 空行分隔头部和正文

    std::string headers_str = headers.str();
    ::send(client_fd_, headers_str.c_str(), headers_str.size(), 0);
    headers_sent_ = true;
}

void Response::send(const std::string &body) {

    std::string response = "HTTP/1.1 " + std::to_string(status_code_) + " " +
                           status_message() + "\r\n";

    headers_["Content-Length"] = std::to_string(body.size());

    for (const auto &[key, value] : headers_) {
        response += key + ": " + value + "\r\n";
    }

    response += "\r\n";

    response += body;

    ::send(client_fd_, response.c_str(), response.size(), 0);
}

Response &Response::end(const std::string &content) {
    if (client_fd_ == -1) {
        spdlog::warn("Response already ended");
        return *this;
    }

    if (!headers_sent_) {
        // 自动设置Content-Length
        if (headers_.find("Content-Length") == headers_.end()) {
            header("Content-Length", std::to_string(content.size()));
        }

        // 自动设置Content-Type
        if (headers_.find("Content-Type") == headers_.end()) {
            header("Content-Type", "text/plain");
        }

        send_headers();
    }

    if (!content.empty()) {
        ::send(client_fd_, content.c_str(), content.size(), 0);
    }

    // 关闭连接
    close(client_fd_);
    client_fd_ = -1;
    return *this;
}

Response &Response::send_file(const std::string &file_path) {
    SPDLOG_DEBUG("send file {}", file_path);
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return status(404).end("File not found");
    }

    // 设置Content-Type
    std::string mime = get_mime_type(file_path);
    if (!mime.empty()) {
        header("Content-Type", mime);
    }

    // 读取文件内容
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    // 发送响应
    header("Content-Length", std::to_string(content.size()));
    send_headers();
    ::send(client_fd_, content.c_str(), content.size(), 0);

    // 关闭连接
    close(client_fd_);
    client_fd_ = -1;
    return *this;
}

std::string Response::get_mime_type(const std::string &path) {
    SPDLOG_INFO("get mime type");
    static const std::unordered_map<std::string, std::string> mime_types = {
        {".html", "text/html"},        {".htm", "text/html"},
        {".css", "text/css"},          {".js", "application/javascript"},
        {".json", "application/json"}, {".xml", "application/xml"},
        {".txt", "text/plain"},        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},       {".png", "image/png"},
        {".gif", "image/gif"},         {".webp", "image/webp"},
        {".svg", "image/svg+xml"},     {".ico", "image/x-icon"},
        {".woff", "font/woff"},        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},          {".otf", "font/otf"},
        {".mp3", "audio/mpeg"},        {".mp4", "video/mp4"},
        {".pdf", "application/pdf"},   {".zip", "application/zip"},
        {".gz", "application/gzip"}};

    size_t dot_pos = path.find_last_of('.');
    if (dot_pos == std::string::npos || dot_pos == path.length() - 1) {
        return "application/octet-stream";
    }

    std::string ext = path.substr(dot_pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto it = mime_types.find(ext);
    if (it != mime_types.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

// 添加目录列表功能
Response &Response::send_directory_listing(const std::string &path,
                                           const std::string &root_dir) {

    SPDLOG_INFO("send directory listing");
    // 生成目录列表 HTML
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Directory "
            "Listing</title></head><body>";
    html << "<h1>Directory Listing: " << path << "</h1><ul>";

    // 获取相对路径（用于生成链接）
    std::string relative_base = fs::relative(path, root_dir).string();
    if (!relative_base.empty() && relative_base.back() != '/') {
        relative_base += '/';
    }

    for (const auto &entry : fs::directory_iterator(path)) {
        std::string filename = entry.path().filename().string();
        std::string ref = "/" + relative_base + filename;

        // 如果是目录，添加斜杠
        if (fs::is_directory(entry.path())) {
            filename += '/';
            ref += '/';
        }

        html << "<li><a href=\"" << ref << "\">" << filename << "</a></li>";
    }

    html << "</ul></body></html>";

    // 发送响应
    status(200).header("Content-Type", "text/html").end(html.str());

    return *this;
}