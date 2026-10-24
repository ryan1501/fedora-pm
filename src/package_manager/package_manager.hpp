#pragma once
#include <string>
#include <vector>
#include "core/process_runner.hpp"

namespace fedora_pm::packages {
struct PackageResult {
    bool success{false};
    std::string output;
    std::string error;
};
class PackageManager {
public:
    explicit PackageManager(std::string executable = "dnf");
    PackageResult search(const std::string& query) const;
    PackageResult info(const std::string& package) const;
    PackageResult list_installed() const;
    PackageResult install(const std::vector<std::string>& packages, bool assume_yes = false) const;
    PackageResult remove(const std::vector<std::string>& packages, bool assume_yes = false) const;
    PackageResult update(bool assume_yes = false) const;
private:
    std::string executable_;
    PackageResult run(const std::vector<std::string>& args) const;
};
}
