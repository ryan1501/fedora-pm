#include "package_manager/changelog_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <sstream>
#include <vector>

namespace fedora_pm::packages {
namespace {
PackageResult execute(const std::string& program, const std::vector<std::string>& args) {
    try {
        const auto result = fedora_pm::core::run_process(program, args, {.timeout = std::chrono::minutes(5)});
        return {!result.timed_out && result.exit_code == 0, result.standard_output,
                result.timed_out ? "Command timed out." : result.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
}
PackageResult ChangelogManager::show(const std::string& package) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    auto result = execute("rpm", {"-q", "--changelog", package});
    if (result.success && !fedora_pm::core::trim(result.output).empty()) {
        result.output = "Changelog for " + package + ":\n" + result.output;
        return result;
    }
    auto fallback = execute("dnf", {"changelog", package});
    if (fallback.success) {
        fallback.output = "Changelog for " + package + ":\n" + fallback.output;
        return fallback;
    }
    return {false, {}, "Could not retrieve changelog for " + package +
        ". RPM reports: " + result.error + " DNF reports: " + fallback.error};
}
PackageResult ChangelogManager::show_recent(const std::string& package, std::size_t entries) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    if (entries == 0 || entries > 500) return {false, {}, "Entry limit must be between 1 and 500."};
    auto result = execute("rpm", {"-q", "--changelog", package});
    if (!result.success) return result;
    std::istringstream input(result.output);
    std::ostringstream output;
    std::string line;
    std::size_t count = 0;
    bool in_entry = false;
    while (std::getline(input, line)) {
        if (!line.empty() && line.front() == '*') {
            if (count >= entries) break;
            ++count;
            in_entry = true;
        }
        if (in_entry) output << line << '\n';
    }
    if (count == 0) return {false, {}, "No changelog entries found for " + package + "."};
    return {true, "Recent changelog for " + package + " (up to " + std::to_string(entries) + " entries):\n" + output.str(), {}};
}
}
