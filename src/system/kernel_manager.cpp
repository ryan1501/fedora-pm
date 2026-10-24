#include "system/kernel_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
PackageResult run(const std::string& program, const std::vector<std::string>& args,
                  std::chrono::milliseconds timeout = std::chrono::minutes(30)) {
    try {
        const auto r = fedora_pm::core::run_process(program, args, {.timeout = timeout});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "Command timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
std::vector<std::string> lines(const std::string& text) {
    std::istringstream input(text);
    std::vector<std::string> result;
    std::string line;
    while (std::getline(input, line)) {
        line = fedora_pm::core::trim(line);
        if (!line.empty()) result.push_back(line);
    }
    return result;
}
std::vector<std::string> packages_for_version(const std::string& requested) {
    const auto r = run("rpm", {"-qa", "--qf", "%{NAME}\\t%{VERSION}\\t%{RELEASE}\\t%{ARCH}\\n", "kernel*"});
    if (!r.success) return {};
    std::vector<std::string> packages;
    for (const auto& line : lines(r.output)) {
        std::istringstream row(line);
        std::string name, version, release, arch;
        if (!(row >> name >> version >> release >> arch)) continue;
        const std::string version_release = version + "-" + release;
        const std::string full_version = version_release + "." + arch;
        // Match a complete VERSION, VERSION-RELEASE, or VERSION-RELEASE.ARCH.
        // Substring globs can accidentally match 6.1 when the user meant 6.10.
        if (requested == version || requested == version_release || requested == full_version)
            packages.push_back(name + "-" + full_version);
    }
    return packages;
}
bool matches_running_kernel(const std::string& requested, const std::string& running) {
    if (requested == running) return true;
    if (requested.find('-') == std::string::npos)
        return running.starts_with(requested + "-");
    return running.starts_with(requested + ".");
}
}
KernelInfo detect_kernels() {
    KernelInfo result;
    std::ifstream proc("/proc/sys/kernel/osrelease");
    std::getline(proc, result.running);
    const auto r = run("rpm", {"-q", "--qf", "%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}\n", "kernel-core"});
    if (r.success) result.installed = lines(r.output);
    return result;
}
PackageResult KernelManager::current() const {
    const auto info = detect_kernels();
    if (info.running.empty()) return {false, {}, "Could not determine the running kernel."};
    return {true, "Current kernel: " + info.running + "\n", {}};
}
PackageResult KernelManager::list_installed() const {
    const auto info = detect_kernels();
    std::ostringstream out;
    if (info.installed.empty()) out << "No installed kernel-core packages found.\n";
    else {
        out << "Installed kernels (" << info.installed.size() << "):\n";
        for (const auto& kernel : info.installed) out << (kernel.find(info.running) != std::string::npos ? " * " : "   ") << kernel << '\n';
    }
    return {true, out.str(), {}};
}
PackageResult KernelManager::list_available() const {
    auto result = run("dnf", {"list", "--available", "kernel*"});
    if (!result.success) return result;
    std::ostringstream out;
    std::size_t count = 0;
    for (const auto& line : lines(result.output)) {
        if (line.find("Available Packages") != std::string::npos || line.starts_with("Last metadata")) continue;
        std::istringstream row(line);
        std::string package;
        row >> package;
        if (package.starts_with("kernel-") && package.find("devel") == std::string::npos &&
            package.find("headers") == std::string::npos) {
            out << package << '\n';
            ++count;
        }
    }
    if (count == 0) out << "No available kernel packages were reported by DNF.\n";
    return {true, out.str(), {}};
}
PackageResult KernelManager::install(std::optional<std::string> version, bool assume_yes) const {
    std::string package = "kernel";
    if (version) {
        if (!fedora_pm::core::is_valid_package_name(*version)) return {false, {}, "Invalid kernel version."};
        package += "-" + *version;
    }
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.push_back(package);
    auto result = run("dnf", args, std::chrono::minutes(60));
    if (result.success) result.output += "\nReboot to use the new kernel.\n";
    return result;
}
PackageResult KernelManager::remove(const std::vector<std::string>& versions, bool assume_yes, bool keep_current) const {
    if (versions.empty()) return {false, {}, "Specify at least one kernel version."};
    const auto running = detect_kernels().running;
    std::vector<std::string> packages;
    for (const auto& version : versions) {
        if (!fedora_pm::core::is_valid_package_name(version)) return {false, {}, "Invalid kernel version: " + version};
        if (keep_current && !running.empty() && matches_running_kernel(version, running)) continue;
        auto matches = packages_for_version(version);
        packages.insert(packages.end(), matches.begin(), matches.end());
    }
    std::sort(packages.begin(), packages.end());
    packages.erase(std::unique(packages.begin(), packages.end()), packages.end());
    if (packages.empty()) return {true, "No matching removable kernel packages found; the running kernel was protected.\n", {}};
    std::vector<std::string> args{"remove"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return run("dnf", args, std::chrono::minutes(60));
}
PackageResult KernelManager::remove_old(std::size_t keep_last, bool assume_yes) const {
    if (keep_last == 0 || keep_last > 20) return {false, {}, "keep-last must be between 1 and 20."};
    auto queried = run("rpm", {"-q", "--qf", "%{INSTALLTIME} %{VERSION}-%{RELEASE}.%{ARCH}\n", "kernel-core"});
    if (!queried.success) return queried;
    struct Kernel { std::uint64_t installed_at; std::string version; };
    std::vector<Kernel> kernels;
    for (const auto& line : lines(queried.output)) {
        std::istringstream row(line);
        Kernel k{};
        if (row >> k.installed_at >> k.version) kernels.push_back(std::move(k));
    }
    std::sort(kernels.begin(), kernels.end(), [](const Kernel& a, const Kernel& b) {
        return a.installed_at > b.installed_at;
    });
    const std::string running = detect_kernels().running;
    std::vector<std::string> targets;
    for (std::size_t i = keep_last; i < kernels.size(); ++i) {
        if (!running.empty() && running == kernels[i].version) continue;
        auto matches = packages_for_version(kernels[i].version);
        targets.insert(targets.end(), matches.begin(), matches.end());
    }
    std::sort(targets.begin(), targets.end());
    targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
    if (targets.empty()) return {true, "No old kernel packages need removal.\n", {}};
    std::vector<std::string> args{"remove"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), targets.begin(), targets.end());
    return run("dnf", args, std::chrono::minutes(60));
}
PackageResult KernelManager::info(std::optional<std::string> version) const {
    const std::string target = version.value_or(detect_kernels().running);
    if (!fedora_pm::core::is_valid_package_name(target)) return {false, {}, "Invalid kernel version."};
    const auto matches = packages_for_version(target);
    if (matches.empty()) return {false, {}, "No installed kernel packages matched " + target + "."};
    std::ostringstream out;
    out << "Kernel version/package match: " << target << '\n';
    for (const auto& package : matches) out << "  " << package << '\n';
    return {true, out.str(), {}};
}
}
