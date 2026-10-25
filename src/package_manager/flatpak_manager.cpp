#include "package_manager/flatpak_manager.hpp"
#include "core/validation.hpp"
#include <chrono>
#include <utility>

namespace fedora_pm::packages {
FlatpakManager::FlatpakManager(std::string executable) : executable_(std::move(executable)) {}
PackageResult FlatpakManager::run(const std::vector<std::string>& args) const {
    try {
        const auto r = fedora_pm::core::run_process(executable_, args, {.timeout = std::chrono::minutes(30)});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "Flatpak command timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}
namespace {
bool valid_app_id(const std::string& app_id) {
    return fedora_pm::core::is_valid_flatpak_app_id(app_id);
}
}
PackageResult FlatpakManager::search(const std::string& query) const {
    if (fedora_pm::core::trim(query).empty()) return {false, {}, "Search query must not be empty."};
    return run({"search", query});
}
PackageResult FlatpakManager::list_installed() const { return run({"list", "--app"}); }
PackageResult FlatpakManager::info(const std::string& app_id) const {
    if (!valid_app_id(app_id)) return {false, {}, "Flatpak application ID must be a valid dotted identifier."};
    return run({"info", app_id});
}
PackageResult FlatpakManager::install(const std::string& app_id, bool assume_yes) const {
    if (!valid_app_id(app_id)) return {false, {}, "Flatpak application ID must be a valid dotted identifier."};
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), {"flathub", app_id});
    return run(args);
}
PackageResult FlatpakManager::remove(const std::string& app_id, bool assume_yes) const {
    if (!valid_app_id(app_id)) return {false, {}, "Flatpak application ID must be a valid dotted identifier."};
    std::vector<std::string> args{"uninstall"};
    if (assume_yes) args.push_back("-y");
    args.push_back(app_id);
    return run(args);
}
PackageResult FlatpakManager::update(bool assume_yes) const {
    std::vector<std::string> args{"update"};
    if (assume_yes) args.push_back("-y");
    return run(args);
}
PackageResult FlatpakManager::setup_flathub() const {
    return run({"remote-add", "--if-not-exists", "flathub", "https://flathub.org/repo/flathub.flatpakrepo"});
}
}
