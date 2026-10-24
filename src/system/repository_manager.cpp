#include "system/repository_manager.hpp"
#include "core/validation.hpp"
#include <chrono>
#include <utility>

namespace fedora_pm::system {
RepositoryManager::RepositoryManager(std::string executable) : executable_(std::move(executable)) {}
fedora_pm::packages::PackageResult RepositoryManager::run(const std::vector<std::string>& args) const {
    try {
        const auto r = fedora_pm::core::run_process(executable_, args, {.timeout = std::chrono::minutes(5)});
        return {!r.timed_out && r.exit_code == 0, r.standard_output, r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
fedora_pm::packages::PackageResult RepositoryManager::list() const { return run({"repolist", "--all"}); }
fedora_pm::packages::PackageResult RepositoryManager::enable(const std::string& id) const {
    if (!fedora_pm::core::is_valid_package_name(id)) return {false, {}, "Invalid repository ID."};
    return run({"config-manager", "setopt", id + ".enabled=1"});
}
fedora_pm::packages::PackageResult RepositoryManager::disable(const std::string& id) const {
    if (!fedora_pm::core::is_valid_package_name(id)) return {false, {}, "Invalid repository ID."};
    return run({"config-manager", "setopt", id + ".enabled=0"});
}
}
