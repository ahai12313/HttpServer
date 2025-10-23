#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <cstddef>
#include <map>
#include <string>
#include <variant>
#include <vector>

class JsonValue {
  public:
    using Null = std::nullptr_t;
    using Bool = bool;
    using String = std::string;
    using Number = double;
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    using Value = std::variant<Null, Bool, String, Number, Array, Object>;

    JsonValue() : value(nullptr) {}
    JsonValue(Null) : value(nullptr) {}
    JsonValue(Bool b) : value(b) {}
    JsonValue(const String &s) : value(s) {}
    JsonValue(const char *s) : value(std::string(s)) {}
    JsonValue(Number n) : value(n) {}
    JsonValue(int n) : value(static_cast<double>(n)) {}
    JsonValue(const Array& a) : value(a) {}
    JsonValue(const Object& o) : value(o) {}

    bool is_null() const {return std::holds_alternative<Null>(value);}
    bool is_bool() const { return std::holds_alternative<Bool>(value); }
    bool is_number() const { return std::holds_alternative<Number>(value); }
    bool is_string() const { return std::holds_alternative<String>(value); }
    bool is_array() const { return std::holds_alternative<Array>(value); }
    bool is_object() const { return std::holds_alternative<Object>(value); }

    Bool as_bool() const {return std::get<Bool>(value);}
    Number as_number() const { return std::get<Number>(value); }
    const String& as_string() const { return std::get<String>(value); }
    const Array& as_array() const { return std::get<Array>(value); }
    const Object& as_object() const { return std::get<Object>(value); }

    static JsonValue parse(const String& json);

  private:
    Value value;

    static JsonValue parse_value(const String& json, size_t& index);
    static void skip_whitespace(const String& json, size_t& index);
    static JsonValue parse_null(const std::string& json, size_t& index);
    static JsonValue parse_bool(const std::string& json, size_t& index);
    static JsonValue parse_number(const std::string& json, size_t& index);
    static JsonValue parse_string(const std::string& json, size_t& index);
    static JsonValue parse_array(const std::string& json, size_t& index);
    static JsonValue parse_object(const std::string& json, size_t& index);
};

#endif