#pragma once

#include "Request.h"
#include "Response.h"
#include "Router.h"
#include <exception>
#include <filesystem>

namespace fs = std::filesystem;

class RequestHandler {
public:
    explicit RequestHandler(const fs::path root_path) : root_path_(root_path) {
        setup_routes();
    }
    
    void handle_request(Request req, Response res) {
        try {
            process_request(req, res);
        } catch (const std::exception& e) {
            handle_error(req, res, e);
        }
    }

private:
    
    void process_request(Request& req, Response& res);
    void handle_error(const Request& req, Response& res, const std::exception& e);
    void handle_get(Request& req, Response& res);
    void handle_post(Request& req, Response& res);

    fs::path root_path_;

    void setup_routes();
    Router router_;
};