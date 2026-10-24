#include "system/cachyos_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
PackageResult run(const std::vector<std::string>& args, std::chrono::milliseconds timeout = std::chrono::minutes(30)) {
    try {
        const auto r = fedora_pm::core::run_process("dnf", args, {.timeout = timeout});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "DNF command timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
std::string repo_name(const std::string& type) {
    if (type == "gcc") return "bieszczaders/kernel-cachyos";
    if (type == "lto") return "bieszczaders/kernel-cachyos-lto";
    if (type == "bore") return "bieszczaders/kernel-cachyos-bore";
    return {};
}
}
PackageResult CachyOSManager::status() const {
    auto result = run({"repolist", "enabled"});
    if (!result.success) return result;
    std::ostringstream out;
    out << "CachyOS repository status:\n";
    for (const auto& type : {"gcc", "lto", "bore"}) {
        const auto id = type == std::string("gcc") ? "kernel-cachyos" :
                        type == std::string("lto") ? "kernel-cachyos-lto" : "kernel-cachyos-bore";
        out << "  " << type << ": " << (result.output.find(id) != std::string::npos ? "enabled" : "not detected") << '\n';
    }
    return {true, out.str(), {}};
}
PackageResult CachyOSManager::list_kernels() const {
    auto result = run({"list", "--available", "kernel-cachyos*"});
    if (!result.success) return result;
    std::istringstream input(result.output);
    std::string line;
    std::vector<std::string> packages;
    while (std::getline(input, line)) {
        std::istringstream row(line);
        std::string package;
        row >> package;
        if (package.starts_with("kernel-cachyos") && package.find("devel") == std::string::npos &&
            package.find("headers") == std::string::npos && package.find("modules") == std::string::npos)
            packages.push_back(package);
    }
    std::sort(packages.begin(), packages.end());
    packages.erase(std::unique(packages.begin(), packages.end()), packages.end());
    std::ostringstream out;
    if (packages.empty()) out << "No CachyOS kernels found. Enable a CachyOS COPR repository first.\n";
    else {
        out << "Available CachyOS kernels (" << packages.size() << "):\n";
        for (const auto& package : packages) out << "  " << package << '\n';
    }
    return {true, out.str(), {}};
}
PackageResult CachyOSManager::enable_repository(const std::string& type, bool assume_yes) const {
    const auto repo = repo_name(type);
    if (repo.empty()) return {false, {}, "Repository type must be gcc, lto, or bore."};
    std::vector<std::string> args{"copr", "enable"};
    if (assume_yes) args.push_back("-y");
    args.push_back(repo);
    return run(args, std::chrono::minutes(10));
}
PackageResult CachyOSManager::disable_repository(const std::string& type, bool assume_yes) const {
    const auto repo = repo_name(type);
    if (repo.empty()) return {false, {}, "Repository type must be gcc, lto, or bore."};
    std::vector<std::string> args{"copr", "disable"};
    if (assume_yes) args.push_back("-y");
    args.push_back(repo);
    return run(args, std::chrono::minutes(10));
}
PackageResult CachyOSManager::check_cpu() const {
    std::ifstream input("/proc/cpuinfo");
    if (!input) return {false, {}, "Could not read /proc/cpuinfo."};
    std::unordered_set<std::string> flags;
    std::string line;
    while (std::getline(input, line)) {
        if (line.starts_with("flags") || line.starts_with("Features")) {
            const auto colon = line.find(':');
            if (colon == std::string::npos) continue;
            std::istringstream values(line.substr(colon + 1));
            std::string flag;
            while (values >> flag) flags.insert(flag);
            break;
        }
    }
    const auto has_all = [&](std::initializer_list<const char*> needed) {
        return std::all_of(needed.begin(), needed.end(), [&](const char* flag) { return flags.contains(flag); });
    };
    const bool v2 = has_all({"cmov","cx16","lahf_lm","popcnt","sse3","ssse3","sse4_1","sse4_2"});
    const bool v3 = v2 && has_all({"avx","avx2","bmi1","bmi2","f16c","fma","lzcnt","movbe","xsave"});
    const bool v4 = v3 && has_all({"avx512f","avx512bw","avx512cd","avx512dq","avx512vl"});
    std::ostringstream out;
    out << "CPU x86-64 microarchitecture levels (based on reported flags):\n"
        << "  x86-64-v2: " << (v2 ? "supported" : "not detected") << '\n'
        << "  x86-64-v3: " << (v3 ? "supported" : "not detected") << '\n'
        << "  x86-64-v4: " << (v4 ? "supported" : "not detected") << '\n'
        << "Note: virtual machines may mask CPU flags.\n";
    return {true, out.str(), {}};
}
PackageResult CachyOSManager::install_kernel(const std::string& kernel_type, const std::string& variant, bool assume_yes) const {
    if (variant != "gcc" && variant != "lto" && variant != "bore")
        return {false, {}, "Kernel variant must be gcc, lto, or bore."};
    if (kernel_type != "default" && kernel_type != "lts" && kernel_type != "rt" && kernel_type != "server")
        return {false, {}, "Kernel type must be default, lts, rt, or server."};
    std::string package = "kernel-cachyos";
    if (kernel_type != "default") package += "-" + kernel_type;
    if (variant != "gcc") package += "-" + variant;
    auto repo_status = run({"repolist", "enabled"});
    if (!repo_status.success) return repo_status;
    const std::string repo_marker = variant == "gcc" ? "kernel-cachyos" :
                                    variant == "lto" ? "kernel-cachyos-lto" : "kernel-cachyos-bore";
    if (repo_status.output.find(repo_marker) == std::string::npos) {
        auto enabled = enable_repository(variant, assume_yes);
        if (!enabled.success) return enabled;
    }
    std::vector<std::string> args{"install"};
    if (assume_yes) args.push_back("-y");
    args.push_back(package);
    args.push_back(package + "-devel-matched");
    auto result = run(args, std::chrono::minutes(60));
    if (result.success) result.output += "\nReboot to use the new CachyOS kernel.\n";
    return result;
}
}
