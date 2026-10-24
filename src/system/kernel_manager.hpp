#pragma once
#include <optional>
#include <string>
#include <vector>
#include "package_manager/package_manager.hpp"

namespace fedora_pm::system {
struct KernelInfo {
    std::string running;
    std::vector<std::string> installed;
};
KernelInfo detect_kernels();

class KernelManager {
public:
    fedora_pm::packages::PackageResult current() const;
    fedora_pm::packages::PackageResult list_installed() const;
    fedora_pm::packages::PackageResult list_available() const;
    fedora_pm::packages::PackageResult install(std::optional<std::string> version = std::nullopt,
                                                bool assume_yes = false) const;
    fedora_pm::packages::PackageResult remove(const std::vector<std::string>& versions,
                                               bool assume_yes = false,
                                               bool keep_current = true) const;
    fedora_pm::packages::PackageResult remove_old(std::size_t keep_last = 2,
                                                   bool assume_yes = false) const;
    fedora_pm::packages::PackageResult info(std::optional<std::string> version = std::nullopt) const;
};
}
