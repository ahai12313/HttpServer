#include "Request.h"
#include <spdlog/spdlog.h>

void Request::info() {
    spdlog::debug("method={}", method);
    spdlog::debug("path={}", path);
    spdlog::debug("version={}", version);
    spdlog::debug("body={}", body_str);
    for (auto &[k, v] : headers) {
        spdlog::debug("{}:{}", k, v);
    }
}

bool Request::isBodyLikelyString() {
    auto it = headers.find("Content-Type");
    if (it == headers.end())
        return false;

    std::string contentType = it->second;

    // 文本类型
    if (contentType.find("text/") == 0)
        return true;
    if (contentType.find("application/json") != std::string::npos)
        return true;
    if (contentType.find("application/xml") != std::string::npos)
        return true;
    if (contentType.find("application/x-www-form-urlencoded") !=
        std::string::npos)
        return true;

    // 二进制类型
    if (contentType.find("image/") == 0)
        return false;
    if (contentType.find("audio/") == 0)
        return false;
    if (contentType.find("video/") == 0)
        return false;
    if (contentType.find("application/octet-stream") != std::string::npos)
        return false;

    // 默认情况下，如果未指定，尝试其他方法
    return false;
}