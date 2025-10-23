#include <vector>
#include <string>
#include <sstream>

std::vector<std::string> split_path(const std::string& path, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream token_stream(path);
    std::string token;
    
    while (std::getline(token_stream, token, delimiter)) {
        if (!token.empty()) {  // 跳过空路径段（如连续分隔符或首尾分隔符）
            tokens.push_back(token);
        }
    }
    return tokens;
}