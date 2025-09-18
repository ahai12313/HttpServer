#include "Request.h"
#include <spdlog/spdlog.h>

void Request::info() {
    spdlog::debug("method {}", method);
    spdlog::debug("path {}", path);
    spdlog::debug("version {}", version);
    spdlog::debug("body {}", body);
    for (auto &[k, v] : headers) {
        spdlog::debug("key={} value={}", k, v);
    }
}