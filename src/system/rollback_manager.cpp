#include "system/rollback_manager.hpp"
#include "core/process_runner.hpp"
#include "core/validation.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
PackageResult run_dnf(const std::vector<std::string>& args, bool privileged = false) {
    try {
        const auto r = fedora_pm::core::run_process("dnf", args,
            {.timeout = std::chrono::minutes(60), .use_sudo = privileged});
        return {!r.timed_out && r.exit_code == 0, r.standard_output,
                r.timed_out ? "DNF transaction operation timed out. " + r.standard_error : r.standard_error};
    } catch (const std::exception& e) { return {false, {}, e.what()}; }
}
bool valid_id(const std::string& id) {
    return !id.empty() && id.size() <= 12 &&
        std::all_of(id.begin(), id.end(), [](unsigned char c) { return std::isdigit(c); }) &&
        id.find_first_not_of('0') != std::string::npos;
}
std::string newest_id(const std::string& output) {
    std::istringstream input(output);
    std::string line;
    while (std::getline(input, line)) {
        line = fedora_pm::core::trim(line);
        if (line.empty() || !std::isdigit(static_cast<unsigned char>(line.front()))) continue;
        std::string token;
        for (char c : line) {
            if (std::isdigit(static_cast<unsigned char>(c))) token.push_back(c);
            else break;
        }
        if (valid_id(token)) return token;
    }
    return {};
}
}
PackageResult RollbackManager::list_history() const { return run_dnf({"history", "list"}); }
PackageResult RollbackManager::rollback(std::string transaction_id, bool assume_yes) const {
    if (!valid_id(transaction_id)) return {false, {}, "Transaction ID must be a positive integer."};
    std::vector<std::string> args{"history", "undo", transaction_id};
    if (assume_yes) args.push_back("-y");
    return run_dnf(args, true);
}
PackageResult RollbackManager::rollback_last(bool assume_yes) const {
    const auto history = list_history();
    if (!history.success) return history;
    const std::string id = newest_id(history.output);
    if (id.empty()) return {false, history.output, "Could not determine the newest DNF transaction ID."};
    auto result = rollback(id, assume_yes);
    if (result.success) result.output = "Undid DNF transaction " + id + ".\n" + result.output;
    return result;
}
}
