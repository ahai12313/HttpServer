#include "HttpServer.h"
#include "Request.h"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <string>

int main(int argc, char *argv[]) {

    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
    SPDLOG_INFO("============density test server============");

    Config config{};

    HttpServer server(config);

    server.set_request_handler([&](Request req, Response res) {
        auto root = server.get_root_path();
        auto relative_path = req.path.substr(1);
        auto absolute_path = fs::path(root) / relative_path;

        SPDLOG_DEBUG("Requested path: {}", absolute_path.string());

        if (absolute_path.lexically_normal().string().find(root) != 0) {
            throw std::runtime_error("Invalid path traversal attempt");
        }

        if (req.method == "GET") {
            if (fs::exists(absolute_path)) {
                if (fs::is_directory(absolute_path)) {
                    res.send_directory_listing(absolute_path,
                                               server.get_root_path());
                } else {
                    res.send_file(absolute_path);
                }
            } else {
                throw std::runtime_error("404 Not Found");
            }
        } else {
            throw std::runtime_error("Method not allowed");
        }
    });

    server.set_error_handler([](const Request &req, Response &res,
                                const std::exception &e) {
        SPDLOG_ERROR("Error handling request {}: {}", req.path, e.what());

        if (e.what() == std::string("404 Not Found")) {
            res.status(404).send("404 Not Found");
        } else if (e.what() == std::string("Method not allowed")) {
            res.status(405).send("405 Method Not Allowed");
        } else if (e.what() == std::string("Invalid path traversal attempt")) {
            res.status(403).send("403 Forbidden");
        } else {
            res.status(500).send("500 Internal Server Error");
        }
    });

    server.start();
    server.run();
    return 0;
}