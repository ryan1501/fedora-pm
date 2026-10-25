#pragma once

#include "package_manager/package_manager.hpp"
#include <optional>
#include <string>

namespace fedora_pm::system {
class DriverManager {
public:
    fedora_pm::packages::PackageResult detect() const;
    fedora_pm::packages::PackageResult status() const;
    fedora_pm::packages::PackageResult check_nvidia() const;
    fedora_pm::packages::PackageResult list_nvidia() const;
    fedora_pm::packages::PackageResult install_nvidia(std::optional<std::string> version = std::nullopt,
                                                       bool cuda = false, bool assume_yes = false) const;
    fedora_pm::packages::PackageResult remove_nvidia(bool assume_yes = false) const;
    fedora_pm::packages::PackageResult install_cuda(bool assume_yes = false) const;
    fedora_pm::packages::PackageResult check_cuda_support() const;
    fedora_pm::packages::PackageResult setup_cuda_development(bool assume_yes = false) const;
};
}
