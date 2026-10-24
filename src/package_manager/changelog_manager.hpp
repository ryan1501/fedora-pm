#pragma once

#include "package_manager/package_manager.hpp"
#include <cstddef>
#include <string>

namespace fedora_pm::packages {
class ChangelogManager {
public:
    PackageResult show(const std::string& package) const;
    PackageResult show_recent(const std::string& package, std::size_t entries = 10) const;
};
}
