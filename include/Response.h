#ifndef RESPONSE_H
#define RESPONSE_H

#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

// 响应类
class Response {
  public:
    Response(int client_fd);
    ~Response();

    Response &status(int code);

    Response &header(const std::string &key, const std::string &value);

    Response &send_file(const std::string &file_path);

    std::string get_mime_type(const std::string &path);

    Response &send_directory_listing(const std::string &path,
                                     const std::string &root_dir);

    void send(const std::string &body_str);
    Response &end(const std::string &content = "");

  private:
    void send_headers();

    int client_fd_;
    int status_code_ = 200;
    std::unordered_map<std::string, std::string> headers_;
    bool headers_sent_ = false;

    std::string status_message() const {
        switch (status_code_) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 304:
            return "Not Modified";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 503:
            return "Service Unavailable";
        default:
            return "Unknown Status";
        }
    }
};

#endif