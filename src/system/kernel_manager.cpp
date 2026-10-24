#include "system/kernel_manager.hpp"
#include "core/process_runner.hpp"
#include <fstream>
#include <sstream>

namespace fedora_pm::system {
KernelInfo detect_kernels() {
    KernelInfo result;
    std::ifstream proc("/proc/sys/kernel/osrelease");
    std::getline(proc, result.running);
    try {
        const auto r = fedora_pm::core::run_process("rpm", {"-q", "--qf", "%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}\n", "kernel-core"});
        if (r.exit_code == 0) {
            std::istringstream lines(r.standard_output);
            std::string line;
            while (std::getline(lines, line)) if (!line.empty()) result.installed.push_back(line);
        }
    } catch (...) { /* Kernel inventory is best-effort; caller can show an empty list. */ }
    return result;
}
}
