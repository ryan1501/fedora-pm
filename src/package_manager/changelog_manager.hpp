#pragma once

#include "package_manager/package_manager.hpp"
#include <cstddef>
#include <string>

namespace fedora_pm::packages {
class ChangelogManager {
public:
    PackageResult show(const std::string& package) const;
    PackageResult show_recent(const std::string& package, std::size_t entries = 10) const;
    PackageResult whats_new(std::size_t limit = 10) const;
    PackageResult compare(const std::string& package, const std::string& version1, const std::string& version2) const;
};
}
