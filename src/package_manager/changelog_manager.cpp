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
PackageResult ChangelogManager::whats_new(std::size_t limit) const {
    if (limit == 0 || limit > 100) return {false, {}, "Update limit must be between 1 and 100."};
    try {
        const auto updates = fedora_pm::core::run_process("dnf", {"check-update", "--quiet"},
            {.timeout = std::chrono::minutes(5)});
        // DNF uses exit code 100 when updates are available.
        if (updates.timed_out) return {false, updates.standard_output, "DNF update check timed out."};
        if (updates.exit_code != 0 && updates.exit_code != 100)
            return {false, updates.standard_output, updates.standard_error.empty()
                ? "DNF could not check for updates." : updates.standard_error};
        std::istringstream input(updates.standard_output);
        std::ostringstream output;
        std::string line;
        std::size_t count = 0;
        while (std::getline(input, line)) {
            const auto trimmed = fedora_pm::core::trim(line);
            if (trimmed.empty() || trimmed.starts_with("Last metadata") ||
                trimmed.starts_with("Obsoleting") || trimmed.starts_with("Security:")) continue;
            std::istringstream fields(trimmed);
            std::string name, version, repository, extra;
            if (!(fields >> name >> version >> repository) || (fields >> extra)) continue;
            if (count == 0) output << "Available updates and installed-package changelog previews:\\n";
            output << "\\n" << (count + 1) << ". " << trimmed << '\\n';
            std::string package = name;
            const auto dot = package.rfind('.');
            if (dot != std::string::npos) {
                const auto arch = package.substr(dot + 1);
                if (arch == "x86_64" || arch == "aarch64" || arch == "noarch" ||
                    arch == "i686" || arch == "i386" || arch == "src") package.resize(dot);
            }
            const auto changelog = execute("rpm", {"-q", "--changelog", package});
            if (changelog.success) {
                std::istringstream changelog_lines(changelog.output);
                std::string changelog_line;
                std::size_t preview_lines = 0;
                while (preview_lines < 5 && std::getline(changelog_lines, changelog_line)) {
                    output << "  " << changelog_line << '\\n';
                    ++preview_lines;
                }
            } else {
                output << "  (No installed-package changelog available.)\\n";
            }
            if (++count >= limit) break;
        }
        if (count == 0) return {true, "No package updates available.\\n", {}};
        if (count == limit) output << "\\nShowing at most " << limit << " updates.\\n";
        return {true, output.str(), {}};
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}

PackageResult ChangelogManager::compare(const std::string& package, const std::string& version1,
                                        const std::string& version2) const {
    if (!fedora_pm::core::is_valid_package_name(package))
        return {false, {}, "Invalid package name."};
    if (fedora_pm::core::trim(version1).empty() || fedora_pm::core::trim(version2).empty())
        return {false, {}, "Both versions must be provided."};
    auto result = show(package);
    if (!result.success) return result;
    result.output = "Changelog comparison for " + package + " (" + version1 + " -> " + version2 + ")\\n" +
        "The installed RPM changelog is shown below; RPM does not provide a reliable arbitrary-version diff here, " +
        "so entries are not claimed to be version-filtered.\\n\\n" + result.output;
    return result;
}
}
