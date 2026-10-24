#pragma once
#include <string>

namespace fedora_pm::system {
struct SystemInfo {
    std::string operating_system;
    std::string version;
    std::string architecture;
    std::string package_manager;
};
SystemInfo detect_system();
}
