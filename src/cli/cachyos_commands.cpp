#include "cli/cachyos_commands.hpp"
#include "system/cachyos_manager.hpp"

#include <string>

namespace fedora_pm::cli {
std::optional<fedora_pm::packages::PackageResult> handle_cachyos_command(int argc, char** argv) {
    if (argc < 2) return std::nullopt;
    const std::string command = argv[1];
    const fedora_pm::system::CachyOSManager manager;
    if (command == "cachyos-status" && argc == 2) return manager.status();
    if (command == "cachyos-list-kernels" && argc == 2) return manager.list_kernels();
    if (command == "cachyos-check-cpu" && argc == 2) return manager.check_cpu();
    if ((command == "cachyos-enable-repo" || command == "cachyos-disable-repo") &&
        (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) return fedora_pm::packages::PackageResult{false, {}, "Usage: fedora-pm cachyos-enable-repo TYPE [--yes]"};
        return command == "cachyos-enable-repo"
            ? manager.enable_repository(argv[2], yes)
            : manager.disable_repository(argv[2], yes);
    }
    if (command == "cachyos-install-kernel" && argc >= 3 && argc <= 5) {
        std::string variant = "gcc";
        bool yes = false;
        if (argc >= 4) {
            if (std::string(argv[argc - 1]) == "--yes") yes = true;
            else variant = argv[3];
        }
        if (argc == 5) {
            if (!yes || std::string(argv[3]) == "--yes") {
                return fedora_pm::packages::PackageResult{false, {}, "Usage: fedora-pm cachyos-install-kernel TYPE [gcc|lto|bore] [--yes]"};
            }
            variant = argv[3];
        }
        return manager.install_kernel(argv[2], variant, yes);
    }
    if (command == "cachyos-status" || command == "cachyos-list-kernels" ||
        command == "cachyos-check-cpu" || command == "cachyos-enable-repo" ||
        command == "cachyos-disable-repo" || command == "cachyos-install-kernel") {
        return fedora_pm::packages::PackageResult{false, {}, "Invalid CachyOS command arguments. Run fedora-pm help."};
    }
    return std::nullopt;
}
}
