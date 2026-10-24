#include "system/doctor.hpp"
#include "core/process_runner.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
struct Check { std::string name; bool ok; std::string detail; };
Check run_check(const std::string& name, const std::string& program, const std::vector<std::string>& args) {
    try {
        const auto r = fedora_pm::core::run_process(program, args, {.timeout = std::chrono::seconds(20)});
        std::string detail = r.standard_output.empty() ? r.standard_error : r.standard_output;
        if (detail.empty()) detail = r.exit_code == 0 ? "completed" : "exit code " + std::to_string(r.exit_code);
        while (!detail.empty() && (detail.back() == '\n' || detail.back() == '\r')) detail.pop_back();
        return {name, !r.timed_out && r.exit_code == 0, detail};
    } catch (const std::exception& e) { return {name, false, e.what()}; }
}
}
PackageResult Doctor::run_checks() const {
    const std::vector<Check> checks{
        run_check("DNF availability", "dnf", {"--version"}),
        run_check("RPM database", "rpm", {"--verifydb"}),
        run_check("Root filesystem", "df", {"-P", "-h", "/"}),
        run_check("System service manager", "systemctl", {"is-system-running"}),
    };
    std::ostringstream output;
    output << "Fedora Package Manager — read-only system doctor\n"
           << "================================================\n";
    std::size_t failed = 0;
    for (const auto& check : checks) {
        output << (check.ok ? "[ OK ] " : "[WARN] ") << check.name << ": " << check.detail << '\n';
        if (!check.ok) ++failed;
    }
    auto flatpak = run_check("Flatpak availability (optional)", "flatpak", {"--version"});
    output << (flatpak.ok ? "[ OK ] " : "[INFO] ") << flatpak.name << ": " << flatpak.detail << '\n';
    output << "\nChecks needing attention: " << failed << '\n'
           << "This diagnostic does not install, remove, or change packages. Review warnings before taking action.\n";
    return {true, output.str(), {}};
}
}
