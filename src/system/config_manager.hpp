#pragma once

#include "package_manager/package_manager.hpp"
#include <filesystem>
#include <string>

namespace fedora_pm::system {
struct AppConfig {
    bool auto_clean{false};
    bool parallel_downloads{true};
    bool fastest_mirror{true};
    bool color_output{true};
    std::filesystem::path history_file;
    std::filesystem::path config_dir;
};
class ConfigManager {
public:
    explicit ConfigManager(std::filesystem::path config_dir = {});
    fedora_pm::packages::PackageResult load();
    fedora_pm::packages::PackageResult save() const;
    fedora_pm::packages::PackageResult show();
    fedora_pm::packages::PackageResult get(const std::string& key);
    fedora_pm::packages::PackageResult set(const std::string& key, const std::string& value);
    const AppConfig& config() const { return config_; }
private:
    AppConfig config_;
    fedora_pm::packages::PackageResult parse_file(const std::filesystem::path& path);
};
}
