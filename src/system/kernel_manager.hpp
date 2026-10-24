#pragma once
#include <string>
#include <vector>

namespace fedora_pm::system {
struct KernelInfo {
    std::string running;
    std::vector<std::string> installed;
};
KernelInfo detect_kernels();
}
