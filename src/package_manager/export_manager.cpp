#include "package_manager/export_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace fedora_pm::packages {
PackageResult ExportManager::export_to_file(const std::string& path) const {
    if (fedora_pm::core::trim(path).empty()) return {false, {}, "Output path must not be empty."};
    try {
        const auto result = fedora_pm::core::run_process("dnf", {"repoquery", "--userinstalled"},
            {.timeout = std::chrono::minutes(5)});
        if (result.timed_out || result.exit_code != 0)
            return {false, result.standard_output, result.standard_error.empty()
                ? "DNF could not enumerate user-installed packages." : result.standard_error};
        std::ofstream output(path, std::ios::trunc);
        if (!output) return {false, {}, "Could not open output file: " + path};
        std::istringstream lines(result.standard_output);
        std::string line;
        std::size_t count = 0;
        while (std::getline(lines, line)) {
            line = fedora_pm::core::trim(line);
            if (!line.empty()) {
                output << line << '\n';
                ++count;
            }
        }
        if (!output) return {false, {}, "Failed while writing package list: " + path};
        return {true, "Exported " + std::to_string(count) + " package entries to " + path + "\n", {}};
    } catch (const std::exception& e) {
        return {false, {}, e.what()};
    }
}

PackageResult ExportManager::import_from_file(const std::string& path, bool assume_yes) const {
    if (fedora_pm::core::trim(path).empty()) return {false, {}, "Input path must not be empty."};
    std::ifstream input(path);
    if (!input) return {false, {}, "Could not open package list: " + path};
    std::vector<std::string> packages;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        line = fedora_pm::core::trim(line);
        if (line.empty() || line.starts_with('#')) continue;
        if (!fedora_pm::core::is_valid_package_name(line))
            return {false, {}, "Invalid package entry on line " + std::to_string(line_number) + ": " + line};
        packages.push_back(line);
    }
    if (packages.empty()) return {false, {}, "Package list contains no package entries."};

    if (!assume_yes) {
        std::cout << "The following " << packages.size() << " packages will be installed:\n";
        for (const auto& package : packages) std::cout << "  " << package << '\n';
        std::cout << "Proceed? [y/N] " << std::flush;
        std::string answer;
        if (!std::getline(std::cin, answer) || (answer != "y" && answer != "Y" &&
            answer != "yes" && answer != "YES")) {
            return {true, "Import cancelled.\n", {}};
        }
    }
    return PackageManager{}.install(packages, true);
}
}
