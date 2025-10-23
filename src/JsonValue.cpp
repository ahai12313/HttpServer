#include "JsonValue.h"
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>

JsonValue JsonValue::parse(const String &json) {

    size_t index = 0;
    JsonValue result = parse_value(json, index);

    skip_whitespace(json, index);

    if (index < json.size())
        throw std::runtime_error("Unexpected content after JSON value");

    return result;
}

JsonValue JsonValue::parse_value(const String &json, size_t &index) {
    skip_whitespace(json, index);

    if (index >= json.size()) {
        throw std::runtime_error("Unexpected end of JSON input");
    }

    char c = json[index];
    switch (c) {
    case '"':
        return parse_string(json, index);
    case 't':
    case 'f':
        return parse_bool(json, index);
    case '[':
        return parse_array(json, index);
    case '{':
        return parse_object(json, index);
    case 'n':
        return parse_null(json, index);
    default:
        if (c == '-' || (c >= '0' && c <= '9')) {
            return parse_number(json, index);
        }
        throw std::runtime_error("Unexpected character: " + std::string(1, c));
    }
}

void JsonValue::skip_whitespace(const String &json, size_t &index) {

    while (index < json.size() &&
           std::isspace(static_cast<char>(json[index]))) {
        index++;
    }
}

JsonValue JsonValue::parse_null(const String &json, size_t &index) {
    if (json.substr(index, 4) == "null") {
        index += 4;
        return JsonValue(nullptr);
    }

    throw std::runtime_error("Expect 'null'");
}

JsonValue JsonValue::parse_bool(const String &json, size_t &index) {
    if (json.substr(index, 4) == "true") {
        index += 4;
        return JsonValue(true);
    }

    if (json.substr(index, 5) == "false") {
        index += 5;
        return JsonValue(false);
    }

    throw std::runtime_error("Expect 'true/false'");
}

JsonValue JsonValue::parse_number(const std::string &json, size_t &index) {
    int start = index;

    if (json[index] == '-') {
        index++;
    }

    if (json[index] == '0') {
        index++;
        if (index < json.size() && std::isdigit(json[index])) {
            throw std::runtime_error("Leading zero in number is not allowed");
        }
    } else if (json[index] >= '1' && json[index] <= '9') {
        index++;
        while (index < json.size() && isdigit(json[index])) {
            index++;
        }
    } else {
        throw std::runtime_error("Invalid number format");
    }

    bool has_fraction = false;
    if (index < json.size() && json[index] == '.') {
        has_fraction = true;
        index++;
        if (index >= json.size() || !std::isdigit(json[index])) {
            throw std::runtime_error(
                "Invalid number format after decimal point");
        }
        while (index < json.size() && std::isdigit(json[index])) {
            index++;
        }
    }

    bool has_exponent = false;
    if (index < json.size() && (json[index] == 'e' || json[index] == 'E')) {
        has_exponent = true;
        index++;
        if (index < json.size() && (json[index] == '+' || json[index] == '-')) {
            index++;
        }
        if (index >= json.size() || !std::isdigit(json[index])) {
            throw std::runtime_error("Invalid number format in exponent");
        }
        while (index < json.size() && std::isdigit(json[index])) {
            index++;
        }
    }

    std::string num_str = json.substr(start, index - start);

    if (num_str == "-0.0" || num_str == "-0") {
        return JsonValue(-0.0);
    }

    try {
        return JsonValue(std::stod(num_str));
    } catch (...) {
        throw std::runtime_error("Invalid number: " + num_str);
    }
}

JsonValue JsonValue::parse_string(const std::string &json, size_t &index) {

    if (json[index] != '"') {
        throw std::runtime_error("Expected '\"' at start of string");
    }
    index++;

    String result;
    while (index < json.size() && json[index] != '"') {
        if (json[index] == '\\') {
            index++;
            if (index >= json.size()) {
                throw std::runtime_error(
                    "Unexpected end of string after escape character");
            }
            switch (json[index]) {
            case '"':
                result += '"';
                break;
            case '\\':
                result += '\\';
                break;
            case '/':
                result += '/';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'u':
                index += 4;
                if (index >= json.size())
                    throw std::runtime_error(
                        "Unexpected end of string after Unicode escape");
                result += '?';
                break;
            default:
                throw std::runtime_error("Invalid escape sequence: \\" +
                                         std::string(1, json[index]));
            }
        } else {
            result += json[index];
        }
        index++;
    }
    if (index >= json.size() || json[index] != '"') {
        throw std::runtime_error("Expected '\"' at end of string");
    }
    index++;
    return JsonValue(result);
}

JsonValue JsonValue::parse_array(const std::string &json, size_t &index) {
    if (json[index] != '[') {
        throw std::runtime_error("Expected '[' at start of array");
    }
    index++;

    Array result;
    skip_whitespace(json, index);

    if (index < json.size() && json[index] == ']') {
        index++;
        return JsonValue(result);
    }

    while (index < json.size()) {
        result.push_back(parse_value(json, index));

        skip_whitespace(json, index);
        if (index < json.size() && json[index] == ',') {
            index++;
            skip_whitespace(json, index);
        } else if (index < json.size() && json[index] == ']') {
            index++;
            break;
        } else {
            throw std::runtime_error("Expected ',' or ']' in array");
        }
    }

    return JsonValue(result);
}

JsonValue JsonValue::parse_object(const String &json, size_t &index) {
    if (json[index] != '{') {
        throw std::runtime_error("Expected '{' at start of object");
    }
    index++;

    Object result;
    skip_whitespace(json, index);

    if (index < json.size() && json[index] == '}') {
        index++;
        return JsonValue(result);
    }

    while (index < json.size()) {

        skip_whitespace(json, index);
        if (index >= json.size() || json[index] != '"') {
            throw std::runtime_error("Expected '\"' at start of object key");
        }
        JsonValue key = parse_string(json, index);

        skip_whitespace(json, index);
        if (index >= json.size() || json[index] != ':') {
            throw std::runtime_error("Expected ':' after object key");
        }
        index++;

        JsonValue value = parse_value(json, index);
        result[key.as_string()] = value;

        skip_whitespace(json, index);
        if (index < json.size() && json[index] == ',') {
            index++;
            skip_whitespace(json, index);
        } else if (index < json.size() && json[index] == '}') {
            index++;
            break;
        } else {
            throw std::runtime_error("Expected ',' or '}' in object");
        }
    }
    return JsonValue(result);
}