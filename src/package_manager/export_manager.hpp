#pragma once

#include <string>
#include "package_manager/package_manager.hpp"

namespace fedora_pm::packages {
class ExportManager {
public:
    PackageResult export_to_file(const std::string& path, bool include_flatpak = false) const;
    PackageResult import_from_file(const std::string& path, bool assume_yes = false) const;
};
}
