#include "RequestHandler.h"

void RequestHandler::setup_routes() {
    // 静态文件服务
    router_.add_route("GET", "/*", [this](Request &req, Response &res) {

        auto relative_path = req.path.substr(1);
        auto absolute_path = root_path_ / relative_path;

        if (fs::exists(absolute_path)) {
            if (fs::is_directory(absolute_path)) {
                res.send_directory_listing(absolute_path, root_path_);
            } else {
                res.send_file(absolute_path);
            }
        } else {
            throw std::runtime_error("404 Not Found");
        }
    });

    // API 端点
    router_.add_route(
        "POST", "/api/v1/commands", [this](Request &req, Response &res) {
            SPDLOG_DEBUG("POST request to: {}", req.path);
            SPDLOG_DEBUG("JSON data: {}", req.body_str);

            if (req.json_value.is_object()) {
                auto server_ip =
                    req.json_value.as_object().at("host").as_string();
                SPDLOG_INFO("Server IP: {}", server_ip);
            }

            if (!req.splited_path.empty() &&
                req.splited_path.back() == "commands") {
                res.status(200).send("Command received\n");
            } else {
                res.status(200).send("POST request processed\n");
            }
        });
}

void RequestHandler::process_request(Request &req, Response &res) {
    auto relative_path = req.path.substr(1);
    auto absolute_path = root_path_ / relative_path;

    SPDLOG_INFO("Requested path: {}", absolute_path.string());

    // 安全路径检查
    if (absolute_path.lexically_normal().string().find(root_path_.string()) !=
        0) {
        throw std::runtime_error("Invalid path traversal attempt");
    }

    if (!router_.route(req, res)) {

        throw std::runtime_error("Method not allowed");

    }
}

void RequestHandler::handle_error(const Request &req, Response &res,
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
}