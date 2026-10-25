#include "package_manager/package_manager.hpp"
#include "core/validation.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace fedora_pm::packages {
PackageManager::PackageManager(std::string executable) : executable_(std::move(executable)) {}
PackageResult PackageManager::run(const std::vector<std::string>& args, bool privileged) const {
    try {
        const auto r = fedora_pm::core::run_process(executable_, args,
            {.timeout = std::chrono::minutes(15), .use_sudo = privileged});
        return { !r.timed_out && r.exit_code == 0, r.standard_output, r.timed_out ? "DNF command timed out. " + r.standard_error : r.standard_error };
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}
namespace {
bool valid_pattern(const std::string& value) {
    return !value.empty() && std::isalnum(static_cast<unsigned char>(value.front())) &&
           std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '+' || c == ':' || c == '*';
    });
}
bool valid_group(const std::string& group) {
    const std::string trimmed = fedora_pm::core::trim(group);
    return !trimmed.empty() && trimmed.front() != '-' &&
           group.find_first_of("\r\n") == std::string::npos;
}
}
PackageResult PackageManager::search(const std::string& query) const {
    if (fedora_pm::core::trim(query).empty()) return {false, {}, "Search query must not be empty."};
    return run({"search", query});
}
PackageResult PackageManager::info(const std::string& package) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    try {
        const auto rpm = fedora_pm::core::run_process("rpm", {"-qi", package}, {.timeout = std::chrono::minutes(2)});
        if (!rpm.timed_out && rpm.exit_code == 0) return {true, rpm.standard_output, {}};
    } catch (...) {}
    return run({"info", package});
}
PackageResult PackageManager::list_installed(const std::string& pattern) const {
    if (!pattern.empty() && !valid_pattern(pattern)) return {false, {}, "Invalid package pattern."};
    std::vector<std::string> args{"list", "--installed"};
    if (!pattern.empty()) args.push_back(pattern);
    return run(args);
}
PackageResult PackageManager::list_available(const std::string& pattern) const {
    if (!pattern.empty() && !valid_pattern(pattern)) return {false, {}, "Invalid package pattern."};
    std::vector<std::string> args{"list", "--available"};
    if (!pattern.empty()) args.push_back(pattern);
    return run(args);
}
PackageResult PackageManager::history() const { return run({"history", "list"}); }
PackageResult PackageManager::dependencies(const std::string& package) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    return run({"repoquery", "--requires", "--resolve", package});
}
PackageResult PackageManager::reverse_dependencies(const std::string& package) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    return run({"repoquery", "--installed", "--whatrequires", package});
}
PackageResult PackageManager::installed_groups() const { return run({"group", "list", "--installed"}); }
PackageResult PackageManager::group_info(const std::string& group) const {
    if (!valid_group(group)) return {false, {}, "Group name must not be empty or contain newlines."};
    return run({"group", "info", group});
}
PackageResult PackageManager::install_group(const std::string& group, bool assume_yes) const {
    if (!valid_group(group)) return {false, {}, "Group name must not be empty or contain newlines."};
    std::vector<std::string> args{"group", "install"};
    if (assume_yes) args.push_back("-y");
    args.push_back(group);
    return run(args, true);
}
PackageResult PackageManager::remove_group(const std::string& group, bool assume_yes) const {
    if (!valid_group(group)) return {false, {}, "Group name must not be empty or contain newlines."};
    std::vector<std::string> args{"group", "remove"};
    if (assume_yes) args.push_back("-y");
    args.push_back(group);
    return run(args, true);
}
PackageResult PackageManager::clean_cache() const { return run({"clean", "all"}, true); }
PackageResult PackageManager::install(const std::vector<std::string>& packages, bool assume_yes) const {
    if (packages.empty()) return {false, {}, "Specify at least one package."};
    for (const auto& p : packages) if (!fedora_pm::core::is_valid_package_name(p)) return {false, {}, "Invalid package name: " + p};
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return run(args, true);
}
PackageResult PackageManager::remove(const std::vector<std::string>& packages, bool assume_yes) const {
    if (packages.empty()) return {false, {}, "Specify at least one package."};
    for (const auto& p : packages) if (!fedora_pm::core::is_valid_package_name(p)) return {false, {}, "Invalid package name: " + p};
    std::vector<std::string> args{"remove"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return run(args, true);
}
PackageResult PackageManager::update(bool assume_yes) const {
    return update({}, assume_yes);
}
PackageResult PackageManager::update(const std::vector<std::string>& packages, bool assume_yes) const {
    for (const auto& package : packages) {
        if (!fedora_pm::core::is_valid_package_name(package))
            return {false, {}, "Invalid package name: " + package};
    }
    std::vector<std::string> args{"upgrade"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return run(args, true);
}
}
