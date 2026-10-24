#include "system/system_info.hpp"
#include <fstream>
#include <string>

namespace fedora_pm::system {
SystemInfo detect_system() {
    SystemInfo info{"Linux", "unknown", "unknown", "dnf"};
    std::ifstream os_release("/etc/os-release");
    std::string line;
    while (std::getline(os_release, line)) {
        if (line.rfind("PRETTY_NAME=", 0) == 0) {
            auto value = line.substr(12);
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.size() - 2);
            info.operating_system = value;
        } else if (line.rfind("VERSION_ID=", 0) == 0) {
            auto value = line.substr(11);
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.size() - 2);
            info.version = value;
        } else if (line.rfind("ID=fedora", 0) == 0) {
            info.package_manager = "dnf";
        }
    }
#if defined(__x86_64__)
    info.architecture = "x86_64";
#elif defined(__aarch64__)
    info.architecture = "aarch64";
#elif defined(__i386__)
    info.architecture = "i686";
#endif
    return info;
}
}
