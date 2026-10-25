#include "system/driver_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
PackageResult run(const std::string& program, const std::vector<std::string>& args, std::chrono::milliseconds timeout = std::chrono::minutes(20), bool privileged = false) {
    try {
        const auto r = fedora_pm::core::run_process(program, args,
            {.timeout = timeout, .use_sudo = privileged});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "Command timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
PackageResult dnf(const std::vector<std::string>& args, bool privileged = false) { return run("dnf", args, std::chrono::minutes(20), privileged); }
std::vector<std::string> rpm_lines(const std::string& pattern) {
    const auto result = run("rpm", {"-qa", pattern});
    std::vector<std::string> packages;
    if (!result.success) return packages;
    std::istringstream input(result.output);
    std::string line;
    while (std::getline(input, line)) {
        line = fedora_pm::core::trim(line);
        if (!line.empty()) packages.push_back(line);
    }
    return packages;
}
}
PackageResult DriverManager::detect() const {
    auto result = run("lspci", {});
    if (!result.success) return {false, {}, "Could not run lspci; install pciutils to detect GPUs. " + result.error};
    std::istringstream lines(result.output);
    std::string line;
    std::ostringstream out;
    std::size_t count = 0;
    while (std::getline(lines, line)) {
        std::string upper = line;
        std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        if (upper.find("VGA COMPATIBLE CONTROLLER") != std::string::npos ||
            upper.find("3D CONTROLLER") != std::string::npos ||
            upper.find("DISPLAY CONTROLLER") != std::string::npos) {
            out << line << '\n';
            ++count;
        }
    }
    if (count == 0) out << "No display/GPU controllers reported by lspci.\n";
    return {true, out.str(), {}};
}
PackageResult DriverManager::status() const {
    const auto gpu = detect();
    const auto nvidia = run("nvidia-smi", {});
    std::ostringstream out;
    out << "=== GPU detection ===\n" << (gpu.success ? gpu.output : gpu.error)
        << "\n=== NVIDIA runtime ===\n";
    if (nvidia.success) out << nvidia.output;
    else out << "nvidia-smi unavailable or driver not working: " << nvidia.error << '\n';
    auto installed = rpm_lines("nvidia*");
    auto akmod = rpm_lines("akmod-nvidia*");
    installed.insert(installed.end(), akmod.begin(), akmod.end());
    std::sort(installed.begin(), installed.end());
    installed.erase(std::unique(installed.begin(), installed.end()), installed.end());
    out << "\nInstalled NVIDIA packages:\n";
    if (installed.empty()) out << "  None detected\n";
    else for (const auto& package : installed) out << "  " << package << '\n';
    return {true, out.str(), {}};
}
PackageResult DriverManager::list_nvidia() const {
    auto a = dnf({"list", "--available", "akmod-nvidia*"});
    auto b = dnf({"list", "--available", "nvidia-driver*"});
    std::ostringstream out;
    out << "Available NVIDIA driver packages (check RPM Fusion repositories):\n";
    if (a.success) out << a.output << '\n';
    if (b.success) out << b.output << '\n';
    if (!a.success && !b.success) return {false, {}, a.error + "\n" + b.error};
    return {true, out.str(), {}};
}
PackageResult DriverManager::install_nvidia(std::optional<std::string> version, bool cuda, bool assume_yes) const {
    std::vector<std::string> packages;
    if (version) {
        if (!fedora_pm::core::is_valid_package_name(*version)) return {false, {}, "Invalid NVIDIA driver version suffix."};
        packages.push_back("akmod-nvidia-" + *version);
    } else packages.push_back("akmod-nvidia");
    packages.push_back("xorg-x11-drv-nvidia");
    if (cuda) {
        packages.push_back("xorg-x11-drv-nvidia-cuda");
        packages.push_back("xorg-x11-drv-nvidia-cuda-libs");
    }
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    auto result = dnf(args, true);
    if (result.success) result.output += "\nReboot after installation and verify with nvidia-smi.\n";
    return result;
}
PackageResult DriverManager::remove_nvidia(bool assume_yes) const {
    auto packages = rpm_lines("nvidia*");
    auto akmod = rpm_lines("akmod-nvidia*");
    packages.insert(packages.end(), akmod.begin(), akmod.end());
    std::sort(packages.begin(), packages.end());
    packages.erase(std::unique(packages.begin(), packages.end()), packages.end());
    if (packages.empty()) return {true, "No NVIDIA packages detected.\n", {}};
    std::vector<std::string> args{"remove"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), packages.begin(), packages.end());
    return dnf(args, true);
}
PackageResult DriverManager::install_cuda(bool assume_yes) const {
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.insert(args.end(), {"cuda", "cuda-toolkit", "nvidia-cuda-toolkit"});
    return dnf(args, true);
}
}
