#include "system/disk_space_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
PackageResult run(const std::string& program, const std::vector<std::string>& args, bool privileged = false) {
    try {
        const auto r = fedora_pm::core::run_process(program, args,
            {.timeout = std::chrono::minutes(5), .use_sudo = privileged});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.standard_error.empty() && r.timed_out ? "Command timed out." : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
struct Entry { unsigned long long bytes; std::string name; };
std::vector<Entry> parse_entries(const std::string& output) {
    std::istringstream stream(output);
    std::vector<Entry> entries;
    std::string line;
    while (std::getline(stream, line)) {
        std::istringstream row(line);
        Entry e{};
        if (row >> e.bytes >> e.name) entries.push_back(std::move(e));
    }
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.bytes > b.bytes;
    });
    return entries;
}
}
PackageResult DiskSpaceManager::top_packages(std::size_t limit) const {
    if (limit == 0 || limit > 1000) return {false, {}, "Limit must be between 1 and 1000."};
    auto result = run("rpm", {"-qa", "--queryformat", "%{SIZE} %{NAME}\n"});
    if (!result.success) return result;
    const auto entries = parse_entries(result.output);
    std::ostringstream out;
    out << "Top " << limit << " largest installed packages:\n";
    out << std::left << std::setw(42) << "Package" << std::right << std::setw(14) << "Size (MiB)" << '\n';
    out << std::string(56, '-') << '\n' << std::fixed << std::setprecision(2);
    for (std::size_t i = 0; i < std::min(limit, entries.size()); ++i)
        out << std::left << std::setw(42) << entries[i].name << std::right
            << std::setw(14) << (static_cast<double>(entries[i].bytes) / (1024.0 * 1024.0)) << '\n';
    return {true, out.str(), {}};
}
PackageResult DiskSpaceManager::package_size(const std::string& package) const {
    if (!fedora_pm::core::is_valid_package_name(package)) return {false, {}, "Invalid package name."};
    auto result = run("rpm", {"-qi", package});
    if (!result.success) return result;
    std::ostringstream out;
    out << "Package details for " << package << ":\n";
    std::istringstream lines(result.output);
    std::string line;
    while (std::getline(lines, line))
        if (line.starts_with("Name") || line.starts_with("Version") || line.starts_with("Installed Size") ||
            line.starts_with("Size") || line.starts_with("Summary")) out << line << '\n';
    auto deps = run("dnf", {"repoquery", "--requires", package});
    if (deps.success) {
        std::size_t count = 0;
        std::istringstream dep_lines(deps.output);
        while (std::getline(dep_lines, line)) if (!fedora_pm::core::trim(line).empty()) ++count;
        out << "Declared dependencies: " << count << '\n';
    }
    return {true, out.str(), {}};
}
PackageResult DiskSpaceManager::cache_size() const {
    auto result = run("du", {"-sh", "/var/cache/dnf"});
    if (result.success) result.output = "DNF cache: " + result.output;
    return result;
}
PackageResult DiskSpaceManager::orphaned_packages() const {
    auto result = run("dnf", {"repoquery", "--extras"});
    if (!result.success) return result;
    std::istringstream lines(result.output);
    std::string line;
    std::ostringstream out;
    std::size_t count = 0;
    while (std::getline(lines, line)) {
        if (fedora_pm::core::trim(line).empty()) continue;
        if (count++ == 0) out << "Packages not available from enabled repositories (review before removal):\n";
        out << "  " << line << '\n';
    }
    if (count == 0) out << "No extra/orphan candidates reported by DNF.\n";
    out << "Candidate count: " << count << '\n';
    return {true, out.str(), {}};
}
PackageResult DiskSpaceManager::remove_orphans(bool assume_yes) const {
    std::vector<std::string> args{"autoremove"};
    if (assume_yes) args.push_back("-y");
    return run("dnf", args, true);
}
PackageResult DiskSpaceManager::total_size() const {
    auto packages = run("rpm", {"-qa", "--queryformat", "%{SIZE}\\n"});
    if (!packages.success) return packages;
    unsigned long long total = 0;
    std::istringstream lines(packages.output);
    std::string line;
    while (std::getline(lines, line)) {
        try { total += std::stoull(line); } catch (...) {}
    }
    std::ostringstream out;
    out << std::fixed << std::setprecision(2)
        << "Total installed package size: "
        << (static_cast<double>(total) / (1024.0 * 1024.0)) << " MB\\n";
    return {true, out.str(), {}};
}
PackageResult DiskSpaceManager::analyze() const {
    auto packages = run("rpm", {"-qa", "--queryformat", "%{SIZE}\n"});
    if (!packages.success) return packages;
    unsigned long long total = 0;
    std::istringstream lines(packages.output);
    std::string line;
    while (std::getline(lines, line)) {
        try { total += std::stoull(line); } catch (...) {}
    }
    std::ostringstream out;
    out << std::fixed << std::setprecision(2)
        << "Installed package payload size: " << (static_cast<double>(total) / (1024.0*1024.0*1024.0)) << " GiB\n";
    auto cache = cache_size();
    if (cache.success) out << cache.output;
    auto df = run("df", {"-h", "/"});
    if (df.success) out << "\nRoot filesystem:\n" << df.output;
    auto orphans = orphaned_packages();
    if (orphans.success) out << "\n" << orphans.output;
    out << "\nNote: RPM package sizes are not the same as actual filesystem usage.\n";
    return {true, out.str(), {}};
}
}
