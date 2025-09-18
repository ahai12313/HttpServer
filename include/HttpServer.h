#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "Request.h"
#include "Response.h"
#include <filesystem>
#include <functional>
#include <string>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

struct Config {
    int port = 8080;
    fs::path root_dir = "www";
    int thread_pool_size = 4;
    int max_connections = 100;
    bool enable_directory_listing = true;
    bool enable_logging = true;
};

class HttpServer {
  public:
    explicit HttpServer(const Config &config = {});
    ~HttpServer();

    HttpServer(const HttpServer &) = delete;
    HttpServer &operator=(const HttpServer &) = delete;

    void start();
    void run();
    void set_root_directory(const std::string &path);

    using RequestHandler = std::function<void(Request, Response)>;
    void set_request_handler(RequestHandler handler);

    using ErrorHandler = std::function<void(const Request& req, Response& res, const std::exception& e)>;
    void set_error_handler(ErrorHandler handler);

    const std::string& get_root_path();

    void update_config(const Config& new_config);

    const Config& config() const;

  private:

    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif