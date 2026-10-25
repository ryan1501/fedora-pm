#include "system/gaming_manager.hpp"
#include "core/process_runner.hpp"

#include <chrono>
#include <vector>

namespace fedora_pm::system {
fedora_pm::packages::PackageResult GamingManager::install_meta(bool assume_yes) const {
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.push_back("fedora-gaming-meta");
    try {
        const auto result = fedora_pm::core::run_process("dnf", args,
            {.timeout = std::chrono::minutes(60), .use_sudo = true});
        if (result.timed_out) return {false, result.standard_output, "Gaming meta-package installation timed out. " + result.standard_error};
        if (result.exit_code != 0) return {false, result.standard_output, result.standard_error};
        return {true, result.standard_output + "\nGaming meta-package installed.\n", {}};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
}
