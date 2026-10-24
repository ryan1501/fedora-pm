#include "core/validation.hpp"
#include <algorithm>
#include <cctype>

namespace fedora_pm::core {
bool is_valid_package_name(std::string_view name) {
    if (name.empty() || name.size() > 256) return false;
    return std::all_of(name.begin(), name.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '+' || c == ':';
    });
}

std::string trim(std::string_view value) {
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    if (first >= last) return {};
    return std::string(first, last);
}
}
