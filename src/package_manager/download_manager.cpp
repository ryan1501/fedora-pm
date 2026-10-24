#include "package_manager/download_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <filesystem>
#include <vector>

namespace fedora_pm::packages {
namespace {
PackageResult execute(const std::vector<std::string>& args, std::chrono::milliseconds timeout = std::chrono::minutes(30)) {
    try {
        const auto result = fedora_pm::core::run_process("dnf", args, {.timeout = timeout});
        if (result.timed_out) return {false, result.standard_output, "DNF command timed out. " + result.standard_error};
        return {result.exit_code == 0, result.standard_output, result.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
bool valid_packages(const std::vector<std::string>& packages) {
    if (packages.empty()) return false;
    for (const auto& package : packages)
        if (!fedora_pm::core::is_valid_package_name(package)) return false;
    return true;
}
PackageResult download_to_impl(const std::vector<std::string>& packages, const std::string& destination, bool resolve) {
    if (!valid_packages(packages))
        return {false, {}, packages.empty() ? "Specify at least one package." : "Invalid package name in download list."};
    if (fedora_pm::core::trim(destination).empty()) return {false, {}, "Destination directory must not be empty."};
    std::error_code ec;
    std::filesystem::create_directories(destination, ec);
    if (ec) return {false, {}, "Could not create destination directory: " + ec.message()};
    std::vector<std::string> args{"download"};
    if (resolve) args.push_back("--resolve");
    args.push_back("--destdir");
    args.push_back(destination);
    args.insert(args.end(), packages.begin(), packages.end());
    auto result = execute(args);
    if (result.success) result.output += "\nDownloaded package files to " + destination + "\n";
    return result;
}
}
PackageResult DownloadManager::download(const std::vector<std::string>& packages) const {
    if (!valid_packages(packages))
        return {false, {}, packages.empty() ? "Specify at least one package." : "Invalid package name in download list."};
    auto result = execute([&] { std::vector<std::string> a{"download"}; a.insert(a.end(), packages.begin(), packages.end()); return a; }());
    if (result.success) result.output += "\nDownloaded package files to the current directory.\n";
    return result;
}
PackageResult DownloadManager::download_to(const std::vector<std::string>& packages, const std::string& destination) const {
    return download_to_impl(packages, destination, false);
}
PackageResult DownloadManager::download_with_dependencies(const std::vector<std::string>& packages,
                                                            const std::string& destination) const {
    return download_to_impl(packages, destination, true);
}
PackageResult DownloadManager::install_offline(const std::vector<std::string>& rpm_files, bool assume_yes) const {
    if (rpm_files.empty()) return {false, {}, "Specify at least one RPM file."};
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    for (const auto& file : rpm_files) {
        const std::filesystem::path path(file);
        if (path.extension() != ".rpm")
            return {false, {}, "Offline installation accepts only .rpm files: " + file};
        std::error_code ec;
        if (!std::filesystem::is_regular_file(path, ec) || ec)
            return {false, {}, "RPM file does not exist or is not a regular file: " + file};
        args.push_back(path.string()};
    }
    return execute(args);
}
}
