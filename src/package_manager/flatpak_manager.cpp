#include "package_manager/flatpak_manager.hpp"
#include "core/validation.hpp"
#include <chrono>
#include <utility>

namespace fedora_pm::packages {
FlatpakManager::FlatpakManager(std::string executable) : executable_(std::move(executable)) {}
PackageResult FlatpakManager::run(const std::vector<std::string>& args) const {
    try {
        const auto r = fedora_pm::core::run_process(executable_, args, {.timeout = std::chrono::minutes(15)});
        return {!r.timed_out && r.exit_code == 0, r.standard_output, r.standard_error};
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}
PackageResult FlatpakManager::search(const std::string& query) const {
    if (fedora_pm::core::trim(query).empty()) return {false, {}, "Search query must not be empty."};
    return run({"search", query});
}
PackageResult FlatpakManager::list_installed() const { return run({"list", "--app"}); }
PackageResult FlatpakManager::install(const std::string& app_id, bool assume_yes) const {
    if (!fedora_pm::core::is_valid_package_name(app_id) || app_id.find('.') == std::string::npos)
        return {false, {}, "Flatpak application ID must be a valid dotted identifier."};
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), {"flathub", app_id});
    return run(args);
}
PackageResult FlatpakManager::remove(const std::string& app_id, bool assume_yes) const {
    if (!fedora_pm::core::is_valid_package_name(app_id) || app_id.find('.') == std::string::npos)
        return {false, {}, "Flatpak application ID must be a valid dotted identifier."};
    std::vector<std::string> args{"uninstall"};
    if (assume_yes) args.push_back("-y");
    args.push_back(app_id);
    return run(args);
}
}
