#pragma once
#include <string>
#include <vector>
#include "package_manager/package_manager.hpp"

namespace fedora_pm::packages {
class FlatpakManager {
public:
    explicit FlatpakManager(std::string executable = "flatpak");
    PackageResult search(const std::string& query) const;
    PackageResult list_installed() const;
    PackageResult info(const std::string& app_id) const;
    PackageResult install(const std::string& app_id, bool assume_yes = false) const;
    PackageResult remove(const std::string& app_id, bool assume_yes = false) const;
    PackageResult update(bool assume_yes = false) const;
    PackageResult setup_flathub() const;
private:
    std::string executable_;
    PackageResult run(const std::vector<std::string>& args) const;
};
}
