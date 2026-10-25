#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace fedora_pm::core {
struct ProcessResult {
    int exit_code{-1};
    bool timed_out{false};
    std::string standard_output;
    std::string standard_error;
};

struct ProcessOptions {
    std::chrono::milliseconds timeout{std::chrono::seconds(60)};
    bool require_success{false};
    // For system-changing operations, use sudo without a shell when not already root.
    bool use_sudo{false};
};

// Executes an argument vector directly (never through a shell), avoiding shell injection.
ProcessResult run_process(const std::string& program,
                          const std::vector<std::string>& arguments,
                          const ProcessOptions& options = {});
}
