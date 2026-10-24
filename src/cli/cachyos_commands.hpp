#pragma once

#include "package_manager/package_manager.hpp"
#include <optional>

namespace fedora_pm::cli {
std::optional<fedora_pm::packages::PackageResult> handle_cachyos_command(int argc, char** argv);
}
