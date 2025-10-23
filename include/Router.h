#pragma once

#include "Request.h"
#include "Response.h"
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <vector>

using Handler = std::function<void(Request&, Response&)>;

class Router {
public:

    void add_route(const std::string& method, const std::string& path, Handler handler);

    bool route(Request& req, Response& res);

    void add_middleware(Handler middleware);

private:

    struct Route {
        std::regex path_pattern;
        Handler handler;
    };

    std::map<std::string, std::vector<Route>> routes_;
    std::vector<Handler> middlewares_;

};