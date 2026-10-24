#pragma once
#include <string>
#include <string_view>

namespace fedora_pm::core {
bool is_valid_package_name(std::string_view name);
bool is_valid_flatpak_app_id(std::string_view app_id);
std::string trim(std::string_view value);
}
