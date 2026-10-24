#include "system/security_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <algorithm>
#include <cctype>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
PackageResult run(const std::vector<std::string>& args, std::chrono::milliseconds timeout = std::chrono::minutes(10)) {
    try {
        const auto r = fedora_pm::core::run_process("dnf", args, {.timeout = timeout});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "DNF command timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
bool valid_advisory(std::string_view value) {
    if (value.empty() || value.size() > 128) return false;
    return std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.';
    });
}
}
PackageResult SecurityManager::list(const std::string& severity) const {
    static const std::vector<std::string> allowed{"security", "critical", "important", "moderate", "low", "bugfix", "enhancement"};
    if (std::find(allowed.begin(), allowed.end(), severity) == allowed.end())
        return {false, {}, "Unsupported severity/category. Use security, critical, important, moderate, low, bugfix, or enhancement."};
    return run({"updateinfo", "list", severity});
}
PackageResult SecurityManager::check() const {
    auto result = list("security");
    if (!result.success) return result;
    if (fedora_pm::core::trim(result.output).empty())
        result.output = "No security update entries were returned by DNF.\n";
    else
        result.output = "DNF security update information:\n" + result.output;
    return result;
}
PackageResult SecurityManager::advisory_info(const std::string& advisory) const {
    if (!valid_advisory(advisory)) return {false, {}, "Invalid advisory identifier."};
    return run({"updateinfo", "info", advisory});
}
PackageResult SecurityManager::cve_check(const std::string& cve) const {
    if (!valid_advisory(cve) || !cve.starts_with("CVE-"))
        return {false, {}, "CVE must use the CVE-YYYY-NNNN identifier format."};
    return run({"updateinfo", "list", "--cve", cve});
}
PackageResult SecurityManager::audit() const {
    PackageResult security = list("security");
    if (!security.success) return security;
    PackageResult critical = list("critical");
    if (!critical.success) return critical;
    PackageResult important = list("important");
    if (!important.success) return important;
    return {true, "=== Security updates ===\n" + security.output +
                  "\n=== Critical updates ===\n" + critical.output +
                  "\n=== Important updates ===\n" + important.output, {}};
}
PackageResult SecurityManager::update(bool assume_yes) const {
    std::vector<std::string> args{"upgrade", "--security"};
    if (assume_yes) args.push_back("-y");
    return run(args, std::chrono::minutes(60));
}
}
