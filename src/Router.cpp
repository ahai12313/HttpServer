#include "Router.h"

#include <regex>
#include <spdlog/spdlog.h>
#include <string>

void Router::add_route(const std::string &method, const std::string &path,
                       Handler handler) {

    std::string patter_str =
        std::regex_replace(path, std::regex("\\:([\\w]+)"), "([^/]+)");
    patter_str = "^" + patter_str + "$";

    routes_[method].push_back({std::regex(patter_str), handler});

    SPDLOG_DEBUG("Added route: {} {}", method, path);
    SPDLOG_DEBUG("patter_str: {}", patter_str);
}

void Router::add_middleware(Handler middleware) {
    middlewares_.push_back(middleware);
}

bool Router::route(Request& req, Response& res)
{
    // for (auto& middleware : middlewares_) {
    //     middleware(req, res);
    //     if (res.sent()) return true; 
    // }

    if (routes_.find(req.method) != routes_.end()) {
        
        for (auto& route : routes_[req.method]) {
            std::smatch matches;
            if (std::regex_match(req.path, matches, route.path_pattern)) {
                for (size_t i = 1; i < matches.size(); i++) {
                    req.params.push_back(matches[i].str());
                }
                route.handler(req, res);
                return true;
            }
        }

    } else {
        SPDLOG_ERROR("request metod not found");
    }

    return false;
}