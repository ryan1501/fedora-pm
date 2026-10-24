#pragma once

#include "package_manager/package_manager.hpp"
#include <string>

namespace fedora_pm::system {
class CachyOSManager {
public:
    fedora_pm::packages::PackageResult status() const;
    fedora_pm::packages::PackageResult list_kernels() const;
    fedora_pm::packages::PackageResult enable_repository(const std::string& type, bool assume_yes = false) const;
    fedora_pm::packages::PackageResult disable_repository(const std::string& type, bool assume_yes = false) const;
    fedora_pm::packages::PackageResult check_cpu() const;
    fedora_pm::packages::PackageResult install_kernel(const std::string& kernel_type,
                                                       const std::string& variant = "gcc",
                                                       bool assume_yes = false) const;
};
}
