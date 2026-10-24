#include "package_manager/export_manager.hpp"
#include "package_manager/flatpak_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace fedora_pm::packages {
PackageResult ExportManager::export_to_file(const std::string& path, bool include_flatpak) const {
    if (fedora_pm::core::trim(path).empty()) return {false, {}, "Output path must not be empty."};
    try {
        const auto result = fedora_pm::core::run_process("dnf", {"repoquery", "--userinstalled"},
            {.timeout = std::chrono::minutes(5)});
        if (result.timed_out || result.exit_code != 0)
            return {false, result.standard_output, result.standard_error.empty()
                ? "DNF could not enumerate user-installed packages." : result.standard_error};

        std::ofstream output(path, std::ios::trunc);
        if (!output) return {false, {}, "Could not open output file: " + path};
        output << "# Fedora Package Manager backup v1\n# DNF Packages\n";
        std::istringstream lines(result.standard_output);
        std::string line;
        std::size_t rpm_count = 0;
        while (std::getline(lines, line)) {
            line = fedora_pm::core::trim(line);
            if (!line.empty()) {
                output << line << '\n';
                ++rpm_count;
            }
        }
        std::size_t flatpak_count = 0;
        if (include_flatpak) try {
            const auto flatpaks = fedora_pm::core::run_process("flatpak",
                {"list", "--app", "--columns=application"}, {.timeout = std::chrono::minutes(2)});
            if (!flatpaks.timed_out && flatpaks.exit_code == 0) {
                output << "\n# Flatpak Applications\n";
                std::istringstream app_lines(flatpaks.standard_output);
                while (std::getline(app_lines, line)) {
                    line = fedora_pm::core::trim(line);
                    if (line.empty() || !fedora_pm::core::is_valid_flatpak_app_id(line))
                        continue;
                    output << "flatpak:" << line << '\n';
                    ++flatpak_count;
                }
            }
        } catch (const std::exception&) {
            // Flatpak is optional; an RPM-only backup is still useful.
        }
        output.close();
        if (!output) return {false, {}, "Failed while writing package list: " + path};
        return {true, "Exported " + std::to_string(rpm_count) + " RPM package entries and " +
            std::to_string(flatpak_count) + " Flatpak applications to " + path + "\n", {}};
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}

PackageResult ExportManager::import_from_file(const std::string& path, bool assume_yes) const {
    if (fedora_pm::core::trim(path).empty()) return {false, {}, "Input path must not be empty."};
    std::ifstream input(path);
    if (!input) return {false, {}, "Could not open package list: " + path};

    std::vector<std::string> packages;
    std::vector<std::string> flatpaks;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        line = fedora_pm::core::trim(line);
        if (line.empty()) continue;

        // Read both the v1 format and backups produced by the old Rust exporter,
        // which stored Flatpak IDs as comments like "# flatpak:org.example.App".
        if (line.starts_with("# flatpak:")) {
            line = fedora_pm::core::trim(line.substr(10));
            if (line.empty()) continue; // Legacy Rust format marker, not an application ID.
        } else if (line.starts_with('#')) continue;
        else if (line.starts_with("flatpak:")) {
            line = fedora_pm::core::trim(line.substr(8));
            if (line.empty())
                return {false, {}, "Empty Flatpak application ID on line " + std::to_string(line_number)};
        }
        else {
            if (!fedora_pm::core::is_valid_package_name(line))
                return {false, {}, "Invalid package entry on line " + std::to_string(line_number) + ": " + line};
            packages.push_back(line);
            continue;
        }

        if (!fedora_pm::core::is_valid_flatpak_app_id(line))
            return {false, {}, "Invalid Flatpak application ID on line " + std::to_string(line_number) + ": " + line};
        flatpaks.push_back(line);
    }
    if (packages.empty() && flatpaks.empty())
        return {false, {}, "Package list contains no package or Flatpak entries."};

    if (!assume_yes) {
        std::cout << "The following " << packages.size() << " RPM packages and " << flatpaks.size()
                  << " Flatpak applications will be installed:\n";
        for (const auto& package : packages) std::cout << "  " << package << '\n';
        for (const auto& app : flatpaks) std::cout << "  flatpak:" << app << '\n';
        std::cout << "Proceed? [y/N] " << std::flush;
        std::string answer;
        if (!std::getline(std::cin, answer) || (answer != "y" && answer != "Y" &&
            answer != "yes" && answer != "YES")) {
            return {true, "Import cancelled.\n", {}};
        }
    }

    std::ostringstream output;
    if (!packages.empty()) {
        const auto rpm_result = PackageManager{}.install(packages, true);
        if (!rpm_result.success) return rpm_result;
        output << rpm_result.output;
    }
    if (!flatpaks.empty()) {
        FlatpakManager flatpak;
        for (const auto& app : flatpaks) {
            const auto result = flatpak.install(app, true);
            if (!result.success) {
                return {false, output.str(), "RPM import may have completed, but Flatpak installation failed for " +
                    app + ": " + result.error};
            }
            output << result.output;
        }
    }
    return {true, "Package import completed.\n" + output.str(), {}};
}
}
