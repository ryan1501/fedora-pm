#pragma once

#include "package_manager/package_manager.hpp"
#include <string>
#include <vector>

namespace fedora_pm::packages {
class DownloadManager {
public:
    PackageResult download(const std::vector<std::string>& packages) const;
    PackageResult download_to(const std::vector<std::string>& packages, const std::string& destination) const;
    PackageResult download_with_dependencies(const std::vector<std::string>& packages,
                                             const std::string& destination) const;
    PackageResult install_offline(const std::vector<std::string>& rpm_files, bool assume_yes = false) const;
};
}
