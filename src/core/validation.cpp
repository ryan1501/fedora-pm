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

bool is_valid_flatpak_app_id(std::string_view app_id) {
    if (app_id.empty() || app_id.size() > 255) return false;
    bool segment_start = true;
    bool has_separator = false;
    for (const unsigned char c : app_id) {
        if (c == '.') {
            if (segment_start) return false;
            segment_start = true;
            has_separator = true;
            continue;
        }
        const bool alpha = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        const bool digit = c >= '0' && c <= '9';
        if (segment_start) {
            if (!alpha) return false;
            segment_start = false;
        } else if (!alpha && !digit && c != '_' && c != '-') {
            return false;
        }
    }
    return has_separator && !segment_start;
}

std::string trim(std::string_view value) {
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    if (first >= last) return {};
    return std::string(first, last);
}
}
