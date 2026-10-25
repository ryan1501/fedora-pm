#pragma once

#include "package_manager/package_manager.hpp"
#include <cstddef>
#include <string>

namespace fedora_pm::system {
class DiskSpaceManager {
public:
    fedora_pm::packages::PackageResult analyze() const;
    fedora_pm::packages::PackageResult total_size() const;
    fedora_pm::packages::PackageResult top_packages(std::size_t limit = 20) const;
    fedora_pm::packages::PackageResult package_size(const std::string& package) const;
    fedora_pm::packages::PackageResult cache_size() const;
    fedora_pm::packages::PackageResult orphaned_packages() const;
    fedora_pm::packages::PackageResult remove_orphans(bool assume_yes = false) const;
};
}
