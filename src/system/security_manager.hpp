#pragma once

#include "package_manager/package_manager.hpp"
#include <string>

namespace fedora_pm::system {
class SecurityManager {
public:
    fedora_pm::packages::PackageResult check() const;
    fedora_pm::packages::PackageResult list(const std::string& severity = "security") const;
    fedora_pm::packages::PackageResult advisory_info(const std::string& advisory) const;
    fedora_pm::packages::PackageResult cve_check(const std::string& cve) const;
    fedora_pm::packages::PackageResult audit() const;
    fedora_pm::packages::PackageResult update(bool assume_yes = false) const;
};
}
