#include "package_manager/package_manager.hpp"
#include "core/validation.hpp"
#include <stdexcept>

namespace fedora_pm::packages {
PackageManager::PackageManager(std::string executable) : executable_(std::move(executable)) {}
PackageResult PackageManager::run(const std::vector<std::string>& args) const {
    try {
        const auto r = fedora_pm::core::run_process(executable_, args, {.timeout = std::chrono::minutes(15)});
        return { !r.timed_out && r.exit_code == 0, r.standard_output, r.standard_error };
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}
PackageResult PackageManager::search(const std::string& query) const {
    if (fedora_pm::core::trim(query).empty()) return {false, {}, "Search query must not be empty."};
    return run({"search", query});
}
PackageResult PackageManager::info(const std::string& package) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    return run({"info", package});
}
PackageResult PackageManager::list_installed() const { return run({"list", "--installed"}); }
PackageResult PackageManager::history() const { return run({"history", "list"}); }
PackageResult PackageManager::install(const std::vector<std::string>& packages, bool assume_yes) const {
    if (packages.empty()) return {false, {}, "Specify at least one package."};
    for (const auto& p : packages) if (!fedora_pm::core::is_valid_package_name(p)) return {false, {}, "Invalid package name: " + p};
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return run(args);
}
PackageResult PackageManager::remove(const std::vector<std::string>& packages, bool assume_yes) const {
    if (packages.empty()) return {false, {}, "Specify at least one package."};
    for (const auto& p : packages) if (!fedora_pm::core::is_valid_package_name(p)) return {false, {}, "Invalid package name: " + p};
    std::vector<std::string> args{"remove"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return run(args);
}
PackageResult PackageManager::update(bool assume_yes) const {
    std::vector<std::string> args{"upgrade"};
    if (assume_yes) args.push_back("-y");
    return run(args);
}
}
