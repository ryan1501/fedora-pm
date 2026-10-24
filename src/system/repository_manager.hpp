#pragma once
#include <string>
#include <vector>
#include "package_manager/package_manager.hpp"

namespace fedora_pm::system {
class RepositoryManager {
public:
    explicit RepositoryManager(std::string executable = "dnf");
    fedora_pm::packages::PackageResult list() const;
    fedora_pm::packages::PackageResult info(const std::string& repo_id) const;
    fedora_pm::packages::PackageResult enable(const std::string& repo_id) const;
    fedora_pm::packages::PackageResult disable(const std::string& repo_id) const;
    fedora_pm::packages::PackageResult add(const std::string& url) const;
    fedora_pm::packages::PackageResult refresh() const;
private:
    std::string executable_;
    fedora_pm::packages::PackageResult run(const std::vector<std::string>& args) const;
};
}
