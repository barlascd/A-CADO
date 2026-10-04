#pragma once
#include <string>
#include <variant>

class Parameter {
public:
    using Value = std::variant<double, int, bool, std::string>;
private:
    std::string name;
    Value value;
public:
    Parameter(const std::string& name, Value value) : name(name), value(std::move(value)) {}
    const std::string& getName() const { return name; }
    const Value& getValue() const { return value; }
    template <typename T> T get() const { return std::get<T>(value); }
    template <typename T> void set(T newValue) { value = newValue; }
    double asDouble() const {
        if (auto d = std::get_if<double>(&value)) return *d;
        if (auto i = std::get_if<int>(&value)) return *i;
        if (auto b = std::get_if<bool>(&value)) return *b ? 1.0 : 0.0;
        return 0.0;
    }
};
