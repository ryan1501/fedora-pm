#pragma once
#include <string>
#include <string_view>

namespace fedora_pm::core {
bool is_valid_package_name(std::string_view name);
std::string trim(std::string_view value);
}
