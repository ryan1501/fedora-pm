#include "system/repository_manager.hpp"
#include "core/validation.hpp"
#include <chrono>
#include <utility>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
RepositoryManager::RepositoryManager(std::string executable) : executable_(std::move(executable)) {}
PackageResult RepositoryManager::run(const std::vector<std::string>& args) const {
    try {
        const auto r = fedora_pm::core::run_process(executable_, args, {.timeout = std::chrono::minutes(10)});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "DNF command timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
PackageResult RepositoryManager::list() const { return run({"repolist", "--all"}); }
PackageResult RepositoryManager::info(const std::string& id) const {
    if (!fedora_pm::core::is_valid_package_name(id)) return {false, {}, "Invalid repository ID."};
    return run({"repoinfo", id});
}
PackageResult RepositoryManager::enable(const std::string& id) const {
    if (!fedora_pm::core::is_valid_package_name(id)) return {false, {}, "Invalid repository ID."};
    return run({"config-manager", "--set-enabled", id});
}
PackageResult RepositoryManager::disable(const std::string& id) const {
    if (!fedora_pm::core::is_valid_package_name(id)) return {false, {}, "Invalid repository ID."};
    return run({"config-manager", "--set-disabled", id});
}
PackageResult RepositoryManager::add(const std::string& url) const {
    if (!(url.starts_with("https://") || url.starts_with("http://")) ||
        url.find_first_of(" \t\r\n") != std::string::npos)
        return {false, {}, "Repository URL must be an HTTP(S) URL without whitespace."};
    return run({"config-manager", "--add-repo", url});
}
PackageResult RepositoryManager::refresh() const {
    auto clean = run({"clean", "metadata"});
    if (!clean.success) return clean;
    auto cache = run({"makecache"});
    if (cache.success) cache.output = clean.output + cache.output;
    return cache;
}
}
