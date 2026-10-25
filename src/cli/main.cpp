#include "core/validation.hpp"
#include "cli/cachyos_commands.hpp"
#include "package_manager/package_manager.hpp"
#include "package_manager/flatpak_manager.hpp"
#include "package_manager/export_manager.hpp"
#include "package_manager/download_manager.hpp"
#include "package_manager/changelog_manager.hpp"
#include "system/system_info.hpp"
#include "system/repository_manager.hpp"
#include "system/kernel_manager.hpp"
#include "system/disk_space_manager.hpp"
#include "system/security_manager.hpp"
#include "system/config_manager.hpp"
#include "system/doctor.hpp"
#include "system/driver_manager.hpp"
#include "system/gaming_manager.hpp"
#include "system/rollback_manager.hpp"
#include "system/history_manager.hpp"
#include <algorithm>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#ifndef FEDORAPM_VERSION
#define FEDORAPM_VERSION "unknown"
#endif

namespace {
void usage() {
    std::cout << "fedora-pm (C++20 migration in progress)\n"
              << "Usage: fedora-pm <command> [arguments]\n"
              << "Options: --version, -V  Show build version\n"
              << "Commands:\n"
              << "  info                              Show OS and architecture\n"
              << "  doctor                            Run read-only system diagnostics\n"
              << "  search QUERY                      Search DNF packages\n"
              << "  info [NAME]                       Show system information or package metadata\n"
              << "  package-info NAME                 Alias for info NAME\n"
              << "  list [--available|--installed|--all] [PATTERN]\n"
              << "  list-available [PATTERN]          List available DNF packages\n"
              << "  clean                             Clean DNF package cache and metadata\n"
              << "  groups                            List installed DNF package groups\n"
              << "  group-info NAME                   Show package group details\n"
              << "  group-install NAME [--yes]        Install a package group\n"
              << "  group-remove NAME [--yes]         Remove a package group\n"
              << "  install [--yes] NAME...           Install RPM packages\n"
              << "  remove [--yes] NAME...            Remove RPM packages\n"
              << "  update [--yes]                    Update installed packages\n"
              << "  history                           Show recent local operation history\n"
              << "  rollback-history                  Show DNF transaction history\n"
              << "  rollback ID [--yes]               Undo a DNF transaction\n"
              << "  rollback-last [--yes]             Undo the newest DNF transaction\n"
              << "  deps NAME                         Show resolved package dependencies\n"
              << "  reverse-deps NAME                 Show installed packages requiring NAME\n"
              << "  groups                            List installed DNF package groups\n"
              << "  changelog NAME                    Show package changelog\n"
              << "  changelog-recent NAME [COUNT]     Show recent installed-package changelog entries\n"
              << "  whats-new [COUNT] | whatsnew [COUNT]  Show available updates and changelog previews\n"
              << "  changelog-compare NAME V1 V2      Show changelog context for two versions\n"
              << "  security-check                    List available security updates\n"
              << "  security-list [SEVERITY]           List security/advisory updates\n"
              << "  security-info ADVISORY             Show advisory details\n"
              << "  cve-check CVE-YYYY-NNNN            Check updates for a CVE\n"
              << "  security-audit                    Summarize security/critical/important updates\n"
              << "  security-update [--yes]            Install security updates\n"
              << "  config show | get KEY | set KEY VALUE\n"
              << "  self-update status | update [--force] [--quiet] | enable FREQUENCY | disable\n"
              << "  gpu-detect                        Detect display controllers\n"
              << "  driver-status                     Show GPU and NVIDIA status\n"
              << "  nvidia-list                       List available NVIDIA driver packages\n"
              << "  nvidia-install [--version V] [--cuda] [--yes]\n"
              << "  nvidia-remove [--yes]             Remove installed NVIDIA packages\n"
              << "  cuda-install [--yes]              Install CUDA packages\n"
              << "  gaming-install [--yes]            Install fedora-gaming-meta\n"
              << "  cachyos-status                    Show CachyOS repository status\n"
              << "  cachyos-list-kernels              List available CachyOS kernels\n"
              << "  cachyos-enable-repo TYPE [--yes]  Enable CachyOS gcc/lto/bore COPR\n"
              << "  cachyos-disable-repo TYPE [--yes] Disable CachyOS gcc/lto/bore COPR\n"
              << "  cachyos-check-cpu                 Check x86-64 CPU feature levels\n"
              << "  cachyos-install-kernel TYPE [VARIANT] [--yes]\n"
              << "  kernel-current                    Show running kernel\n"
              << "  kernel-list                       List installed kernels\n"
              << "  kernel-available                  List available kernels\n"
              << "  kernel-install [VERSION] [--yes]  Install a kernel\n"
              << "  kernel-remove [--yes] VERSION...  Remove kernels (protect running kernel)\n"
              << "  kernel-remove-old [--keep-last N] [--yes]\n"
              << "  kernel-info [VERSION]             Show matching kernel packages\n"
              << "  export [--with-flatpak] FILE     Export installed RPM packages, optionally Flatpaks\n"
              << "  import [--yes] FILE               Import and install a package list\n"
              << "  size [--top N]                    Show largest installed packages\n"
              << "  package-size NAME                 Show package size and metadata\n"
              << "  diskspace                         Analyze package/cache/filesystem usage\n"
              << "  orphans                           List DNF extra-package candidates\n"
              << "  clean-orphans [--yes]             Run DNF autoremove\n"
              << "  download [--to DIR] [--with-deps] PACKAGE...\n"
              << "  install-offline [--yes] FILE.rpm... Install local RPM files\n"
              << "  repo-list                         List DNF repositories\n"
              << "  repo-info ID                      Show repository details\n"
              << "  repo-enable ID | repo-disable ID  Change repository state\n"
              << "  repo-add [NAME] URL               Add a repository URL\n"
              << "  repo-remove ID                    Disable and remove a repository file\n"
              << "  repo-refresh                      Refresh repository metadata\n"
              << "  flatpak-search QUERY              Search Flatpak remotes\n"
              << "  flatpak-list                      List installed Flatpaks\n"
              << "  flatpak-info APP_ID               Show Flatpak details\n"
              << "  flatpak-update [--yes]            Update installed Flatpaks\n"
              << "  flatpak-setup-flathub             Add the Flathub remote if missing\n"
              << "  flatpak-install APP_ID [--yes]    Install a Flatpak\n"
              << "  flatpak-remove APP_ID [--yes]     Remove a Flatpak\n"
              << "  help                              Show this help\n";
}
int print_result(const fedora_pm::packages::PackageResult& result) {
    if (!result.output.empty()) std::cout << result.output;
    if (!result.error.empty()) std::cerr << result.error << (result.error.back() == '\n' ? "" : "\n");
    return result.success ? 0 : 1;
}
int print_mutation_result(const fedora_pm::packages::PackageResult& result,
                          const std::string& action, const std::vector<std::string>& items) {
    const int code = print_result(result);
    if (result.success) {
        const auto logged = fedora_pm::system::HistoryManager{}.log(action, items);
        if (!logged.success) std::cerr << "Warning: operation succeeded but history logging failed: " << logged.error << '\n';
    }
    return code;
}
bool has_yes(int argc, char** argv, int index) {
    return index < argc && std::string(argv[index]) == "--yes";
}
}
int main(int argc, char** argv) {
    // Translate established Rust CLI subcommands to the equivalent C++ commands.
    // Keep this compatibility shim conservative: unsupported legacy workflows
    // must remain visibly unsupported rather than being mapped to a different action.
    std::vector<std::string> normalized_args;
    normalized_args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) normalized_args.emplace_back(argv[i]);
    if (normalized_args.size() >= 3) {
        const std::string top = normalized_args[1];
        const std::string sub = normalized_args[2];
        std::string replacement;
        if (top == "kernel") {
            if (sub == "list") replacement = "kernel-list";
            else if (sub == "install") replacement = "kernel-install";
            else if (sub == "remove") replacement = "kernel-remove";
            else if (sub == "remove-old") replacement = "kernel-remove-old";
            else if (sub == "info") replacement = "kernel-info";
        } else if (top == "driver") {
            if (sub == "status" || sub == "check-nvidia") replacement = "driver-status";
            else if (sub == "detect") replacement = "gpu-detect";
            else if (sub == "list-nvidia") replacement = "nvidia-list";
            else if (sub == "remove-nvidia") replacement = "nvidia-remove";
            else if (sub == "install-nvidia") {
                replacement = "nvidia-install";
                if (normalized_args.size() >= 4 && !normalized_args[3].starts_with("--") &&
                    normalized_args[3] != "-y" && normalized_args[3] != "-q") {
                    normalized_args.insert(normalized_args.begin() + 3, "--version");
                }
                for (auto& arg : normalized_args) if (arg == "--toolkit") arg = "--cuda";
            }
        } else if (top == "gaming" && sub == "install") {
            replacement = "gaming-install";
        } else if (top == "cachyos") {
            if (sub == "status") replacement = "cachyos-status";
            else if (sub == "list-kernels") replacement = "cachyos-list-kernels";
            else if (sub == "install-kernel") replacement = "cachyos-install-kernel";
            else if (sub == "enable-repo") replacement = "cachyos-enable-repo";
            else if (sub == "disable-repo") replacement = "cachyos-disable-repo";
            else if (sub == "check-cpu") replacement = "cachyos-check-cpu";
        } else if (top == "flatpak") {
            if (sub == "search") replacement = "flatpak-search";
            else if (sub == "list") replacement = "flatpak-list";
            else if (sub == "info") replacement = "flatpak-info";
            else if (sub == "install") replacement = "flatpak-install";
            else if (sub == "remove") replacement = "flatpak-remove";
            else if (sub == "update") replacement = "flatpak-update";
            else if (sub == "setup-flathub") replacement = "flatpak-setup-flathub";
        } else if (top == "repo") {
            if (sub == "list") replacement = "repo-list";
            else if (sub == "info") replacement = "repo-info";
            else if (sub == "enable") replacement = "repo-enable";
            else if (sub == "disable") replacement = "repo-disable";
            else if (sub == "refresh") replacement = "repo-refresh";
            else if (sub == "add") replacement = "repo-add";
            else if (sub == "remove") replacement = "repo-remove";
        } else if (top == "security") {
            if (sub == "check") replacement = "security-check";
            else if (sub == "list") replacement = "security-list";
            else if (sub == "update") replacement = "security-update";
            else if (sub == "info") replacement = "security-info";
            else if (sub == "cve") replacement = "cve-check";
            else if (sub == "audit") replacement = "security-audit";
        } else if (top == "group") {
            if (sub == "list") replacement = "groups";
            else if (sub == "info") replacement = "group-info";
            else if (sub == "install") replacement = "group-install";
            else if (sub == "remove") replacement = "group-remove";
        }
        if (!replacement.empty()) {
            normalized_args[1] = replacement;
            normalized_args.erase(normalized_args.begin() + 2);
        }
    }
    // Clap accepted the conventional short confirmation flag; keep it working.
    for (auto& arg : normalized_args) if (arg == "-y") arg = "--yes";
    if (normalized_args.size() >= 4 && normalized_args[1] == "deps") {
        const auto reverse = std::find(normalized_args.begin() + 2, normalized_args.end(), "--reverse");
        if (reverse != normalized_args.end()) {
            normalized_args[1] = "reverse-deps";
            normalized_args.erase(reverse);
        }
    }
    std::vector<char*> normalized_argv;
    normalized_argv.reserve(normalized_args.size());
    for (auto& arg : normalized_args) normalized_argv.push_back(arg.data());
    argc = static_cast<int>(normalized_argv.size());
    argv = normalized_argv.data();
    if (argc == 2 && (std::string(argv[1]) == "--version" ||
                      std::string(argv[1]) == "-V" ||
                      std::string(argv[1]) == "version")) {
        std::cout << "fedora-pm " << FEDORAPM_VERSION << "\n";
        return 0;
    }
    if (argc < 2) { usage(); return 0; }
    if (std::string(argv[1]) == "help" || std::string(argv[1]) == "--help") {
        if (argc != 2) { std::cerr << "Usage: fedora-pm help\n"; return 2; }
        usage();
        return 0;
    }
    const std::string command = argv[1];
    if (command == "info" && argc == 2) {
        const auto s = fedora_pm::system::detect_system();
        std::cout << "OS: " << s.operating_system << "\nVersion: " << s.version
                  << "\nArchitecture: " << s.architecture << "\nPackage manager: " << s.package_manager << "\n";
        return 0;
    }
    if (command == "doctor" && argc == 2) return print_result(fedora_pm::system::Doctor{}.run_checks());
    if (auto cachyos_result = fedora_pm::cli::handle_cachyos_command(argc, argv); cachyos_result.has_value()) {
        if (command == "cachyos-enable-repo" || command == "cachyos-disable-repo" || command == "cachyos-install-kernel") {
            std::vector<std::string> items;
            for (int i = 2; i < argc; ++i) items.emplace_back(argv[i]);
            return print_mutation_result(*cachyos_result, command, items);
        }
        return print_result(*cachyos_result);
    }

    fedora_pm::system::DriverManager drivers;
    if (command == "gpu-detect" && argc == 2) return print_result(drivers.detect());
    if (command == "driver-status" && argc == 2) return print_result(drivers.status());
    if (command == "nvidia-list" && argc == 2) return print_result(drivers.list_nvidia());
    if (command == "nvidia-remove" || command == "cuda-install") {
        if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--yes")) {
            std::cerr << "Usage: fedora-pm " << command << " [--yes]\n";
            return 2;
        }
        return command == "nvidia-remove"
            ? print_mutation_result(drivers.remove_nvidia(has_yes(argc, argv, 2)), "driver_remove_nvidia", {})
            : print_mutation_result(drivers.install_cuda(has_yes(argc, argv, 2)), "driver_install_cuda", {});
    }
    if (command == "nvidia-install") {
        bool yes = false, cuda = false;
        std::optional<std::string> version;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--yes") yes = true;
            else if (arg == "--cuda") cuda = true;
            else if (arg == "--version" && i + 1 < argc) version = argv[++i];
            else { std::cerr << "Usage: fedora-pm nvidia-install [--version V] [--cuda] [--yes]\n"; return 2; }
        }
        return print_mutation_result(drivers.install_nvidia(version, cuda, yes), "driver_install_nvidia", version ? std::vector<std::string>{*version} : std::vector<std::string>{"default"});
    }
    if (command == "gaming-install") {
        if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--yes")) {
            std::cerr << "Usage: fedora-pm gaming-install [--yes]\n";
            return 2;
        }
        return print_mutation_result(fedora_pm::system::GamingManager{}.install_meta(has_yes(argc, argv, 2)), "gaming_install", {"fedora-gaming-meta"});
    }

    fedora_pm::system::KernelManager kernels;
    if (command == "kernel-current" && argc == 2) return print_result(kernels.current());
    if (command == "kernel-list" && argc == 2) return print_result(kernels.list_installed());
    if (command == "kernel-available" && argc == 2) return print_result(kernels.list_available());
    if (command == "kernel-info" && (argc == 2 || argc == 3))
        return print_result(kernels.info(argc == 3 ? std::optional<std::string>(argv[2]) : std::nullopt));
    if (command == "kernel-install" && (argc == 2 || argc == 3 || argc == 4)) {
        bool yes = false;
        std::optional<std::string> version;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--yes") yes = true;
            else if (arg.starts_with("-") || arg.empty() || version) {
                std::cerr << "Usage: fedora-pm kernel-install [VERSION] [--yes]\n";
                return 2;
            } else {
                version = arg;
            }
        }
        return print_mutation_result(kernels.install(version, yes), "kernel_install", {version.value_or("kernel")});
    }
    if (command == "kernel-remove") {
        bool yes = false;
        std::vector<std::string> versions;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--yes") yes = true;
            else if (arg.starts_with("-") || arg.empty()) {
                std::cerr << "Usage: fedora-pm kernel-remove [--yes] VERSION...\n";
                return 2;
            } else {
                versions.emplace_back(arg);
            }
        }
        return print_mutation_result(kernels.remove(versions, yes, true), "kernel_remove", versions);
    }
    if (command == "kernel-remove-old") {
        bool yes = false;
        std::size_t keep = 2;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--yes") yes = true;
            else if (arg == "--keep-last" && i + 1 < argc) {
                const std::string value = argv[++i];
                const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), keep);
                if (ec != std::errc{} || ptr != value.data() + value.size()) {
                    std::cerr << "--keep-last requires a positive integer\n";
                    return 2;
                }
            } else { std::cerr << "Usage: fedora-pm kernel-remove-old [--keep-last N] [--yes]\n"; return 2; }
        }
        if (keep == 0) { std::cerr << "--keep-last requires a positive integer\n"; return 2; }
        return print_mutation_result(kernels.remove_old(keep, yes), "kernel_remove_old", {std::to_string(keep)});
    }

    fedora_pm::system::RollbackManager rollback;
    if (command == "rollback-history" && argc == 2) return print_result(rollback.list_history());
    if (command == "rollback-last") {
        if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--yes")) {
            std::cerr << "Usage: fedora-pm rollback-last [--yes]\n";
            return 2;
        }
        return print_mutation_result(rollback.rollback_last(has_yes(argc, argv, 2)), "rollback_last", {});
    }
    if (command == "rollback" && (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) { std::cerr << "Usage: fedora-pm rollback ID [--yes]\n"; return 2; }
        return print_mutation_result(rollback.rollback(argv[2], yes), "rollback", {argv[2]});
    }

    fedora_pm::packages::PackageManager pm;
    if (command == "history" && argc == 2) return print_result(fedora_pm::system::HistoryManager{}.print_recent(10));
    if (command == "rollback-history") {
        std::cerr << "Usage: fedora-pm rollback-history\n";
        return 2;
    }
    if (command == "search") {
        if (argc != 3) { std::cerr << "Usage: fedora-pm search QUERY\n"; return 2; }
        return print_result(pm.search(argv[2]));
    }
    // Keep the original Rust CLI spelling (info PACKAGE) while retaining the
    // C++ migration's explicit package-info alias and bare system-info command.
    if ((command == "info" || command == "package-info") && argc == 3)
        return print_result(pm.info(argv[2]));
    if (command == "list" || command == "list-available") {
        bool available = command == "list-available";
        bool installed = command == "list";
        bool all = false;
        std::string pattern;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--available") { available = true; installed = false; }
            else if (arg == "--installed") { installed = true; available = false; }
            else if (arg == "--all") { all = true; installed = true; available = true; }
            else if (arg.starts_with("--")) { std::cerr << "Unknown list option: " << arg << '\n'; return 2; }
            else if (pattern.empty()) pattern = arg;
            else { std::cerr << "Usage: fedora-pm list [--available|--installed|--all] [PATTERN]\n"; return 2; }
        }
        if (all) {
            const auto first = pm.list_installed(pattern);
            const auto second = pm.list_available(pattern);
            return print_result({first.success && second.success, first.output + "\n" + second.output, first.error + second.error});
        }
        if (available) return print_result(pm.list_available(pattern));
        return print_result(pm.list_installed(pattern));
    }
    if (command == "clean" && argc == 2) return print_mutation_result(pm.clean_cache(), "clean", {"dnf"});
    if (command == "group-info" && argc == 3) return print_result(pm.group_info(argv[2]));
    if (command == "group-install" && (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) { std::cerr << "Usage: fedora-pm group-install NAME [--yes]\n"; return 2; }
        return print_mutation_result(pm.install_group(argv[2], yes), "group-install", {argv[2]});
    }
    if (command == "group-remove" && (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) { std::cerr << "Usage: fedora-pm group-remove NAME [--yes]\n"; return 2; }
        return print_mutation_result(pm.remove_group(argv[2], yes), "group-remove", {argv[2]});
    }
    if (command == "deps" && argc == 3) return print_result(pm.dependencies(argv[2]));
    if (command == "reverse-deps" && argc == 3) return print_result(pm.reverse_dependencies(argv[2]));
    if (command == "groups" && argc == 2) return print_result(pm.installed_groups());

    fedora_pm::packages::ChangelogManager changelogs;
    if (command == "changelog" && argc == 3) return print_result(changelogs.show(argv[2]));
    if (command == "changelog-recent" && (argc == 3 || argc == 4)) {
        std::size_t count = 10;
        if (argc == 4) {
            const std::string value = argv[3];
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), count);
            if (ec != std::errc{} || ptr != value.data() + value.size() || count == 0) {
                std::cerr << "COUNT must be a positive integer\n";
                return 2;
            }
        }
        return print_result(changelogs.show_recent(argv[2], count));
    }
    if ((command == "whats-new" || command == "whatsnew") && (argc == 2 || argc == 3)) {
        std::size_t count = 10;
        if (argc == 3) {
            const std::string value = argv[2];
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), count);
            if (ec != std::errc{} || ptr != value.data() + value.size() || count == 0 || count > 100) {
                std::cerr << "COUNT must be between 1 and 100\n"; return 2;
            }
        }
        return print_result(changelogs.whats_new(count));
    }
    if (command == "changelog-compare" && argc == 5)
        return print_result(changelogs.compare(argv[2], argv[3], argv[4]));

    fedora_pm::system::SecurityManager security;
    if (command == "security-check" && argc == 2) return print_result(security.check());
    if (command == "security-list" && (argc == 2 || argc == 3))
        return print_result(security.list(argc == 3 ? argv[2] : "security"));
    if (command == "security-info" && argc == 3) return print_result(security.advisory_info(argv[2]));
    if (command == "cve-check" && argc == 3) return print_result(security.cve_check(argv[2]));
    if (command == "security-audit" && argc == 2) return print_result(security.audit());
    if (command == "security-update") {
        if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--yes")) {
            std::cerr << "Usage: fedora-pm security-update [--yes]\n";
            return 2;
        }
        return print_mutation_result(security.update(has_yes(argc, argv, 2)), "security_update", {"security"});
    }

    if (command == "self-update") {
        if (argc == 3 && std::string(argv[2]) == "status") {
            std::cout << "Self-update status: unavailable\n"
                      << "Current version: " << FEDORAPM_VERSION << "\n"
                      << "Update source: GitHub (not configured)\n";
            return 0;
        }
        if (argc >= 3 && std::string(argv[2]) == "update") {
            for (int i = 3; i < argc; ++i) {
                const std::string arg = argv[i];
                if (arg == "--force" || arg == "-f" || arg == "--quiet" || arg == "-q") continue;
                std::cerr << "Usage: fedora-pm self-update update [--force] [--quiet]\n";
                return 2;
            }
            std::cerr << "Self-update is not configured for this C++ build; no changes were made.\n";
            return 1;
        }
        if (argc == 4 && std::string(argv[2]) == "enable") {
            std::cerr << "Automatic self-updates are not implemented in this C++ build.\n";
            return 1;
        }
        if (argc == 3 && std::string(argv[2]) == "disable") {
            std::cerr << "Automatic self-updates are not implemented in this C++ build.\n";
            return 1;
        }
        std::cerr << "Usage: fedora-pm self-update status | update [--force] [--quiet] | enable FREQUENCY | disable\n";
        return 2;
    }

    if (command == "config") {
        fedora_pm::system::ConfigManager config;
        if (argc == 3 && std::string(argv[2]) == "show") return print_result(config.show());
        if (argc == 4 && std::string(argv[2]) == "get") return print_result(config.get(argv[3]));
        if (argc == 5 && std::string(argv[2]) == "set") return print_result(config.set(argv[3], argv[4]));
        std::cerr << "Usage: fedora-pm config show | get KEY | set KEY VALUE\n";
        return 2;
    }

    if (command == "export") {
        bool include_flatpak = false;
        std::string path;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--with-flatpak") include_flatpak = true;
            else if (arg.starts_with("--") || !path.empty()) {
                std::cerr << "Usage: fedora-pm export [--with-flatpak] FILE\n";
                return 2;
            } else path = arg;
        }
        if (path.empty()) {
            std::cerr << "Usage: fedora-pm export [--with-flatpak] FILE\n";
            return 2;
        }
        return print_result(fedora_pm::packages::ExportManager{}.export_to_file(path, include_flatpak));
    }
    if (command == "import") {
        const bool yes = has_yes(argc, argv, 2);
        const int path_index = yes ? 3 : 2;
        if (argc != path_index + 1) {
            std::cerr << "Usage: fedora-pm import [--yes] FILE\n";
            return 2;
        }
        return print_mutation_result(fedora_pm::packages::ExportManager{}.import_from_file(argv[path_index], yes), "import", {argv[path_index]});
    }

    fedora_pm::system::DiskSpaceManager disk;
    if (command == "diskspace") {
        if (argc != 2) { std::cerr << "Usage: fedora-pm diskspace\n"; return 2; }
        return print_result(disk.analyze());
    }
    if (command == "orphans") {
        if (argc != 2) { std::cerr << "Usage: fedora-pm orphans\n"; return 2; }
        return print_result(disk.orphaned_packages());
    }
    if (command == "clean-orphans") {
        if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--yes")) {
            std::cerr << "Usage: fedora-pm clean-orphans [--yes]\n";
            return 2;
        }
        return print_mutation_result(disk.remove_orphans(has_yes(argc, argv, 2)), "clean-orphans", {"autoremove"});
    }
    if (command == "package-size" && argc == 3) return print_result(disk.package_size(argv[2]));
    if (command == "size") {
        std::size_t limit = 20;
        if (argc == 4 && std::string(argv[2]) == "--top") {
            const std::string value = argv[3];
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), limit);
            if (ec != std::errc{} || ptr != value.data() + value.size() || limit == 0) {
                std::cerr << "--top requires a positive integer\n";
                return 2;
            }
        } else if (argc != 2) {
            std::cerr << "Usage: fedora-pm size [--top N]\n";
            return 2;
        }
        return print_result(disk.top_packages(limit));
    }

    fedora_pm::packages::DownloadManager downloads;
    if (command == "download") {
        bool with_deps = false;
        std::string destination = ".";
        std::vector<std::string> packages;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--with-deps" || arg == "-w") {
                with_deps = true;
            } else if (arg == "--to" || arg == "--dest" || arg == "-d") {
                if (i + 1 >= argc || argv[i + 1][0] == '-') {
                    std::cerr << arg << " requires a destination directory\\n";
                    return 2;
                }
                destination = argv[++i];
            } else if (arg.starts_with("-")) {
                std::cerr << "Unknown download option: " << arg << '\\n';
                return 2;
            } else {
                packages.emplace_back(arg);
            }
        }
        if (packages.empty()) {
            std::cerr << "Usage: fedora-pm download [--dest DIR] [--with-deps] PACKAGE...\\n";
            return 2;
        }
        if (with_deps) return print_mutation_result(downloads.download_with_dependencies(packages, destination), "download-with-deps", packages);
        if (destination != ".") return print_mutation_result(downloads.download_to(packages, destination), "download", packages);
        return print_mutation_result(downloads.download(packages), "download", packages);
    }

    if (command == "install-offline") {
        int i = 2;
        bool yes = false;
        if (i < argc && std::string(argv[i]) == "--yes") { yes = true; ++i; }
        if (i >= argc) {
            std::cerr << "Usage: fedora-pm install-offline [--yes] FILE.rpm...\n";
            return 2;
        }
        std::vector<std::string> files;
        for (; i < argc; ++i) {
            const std::string file = argv[i];
            if (file.starts_with("-")) {
                std::cerr << "Unknown install-offline option: " << file << '\n';
                return 2;
            }
            files.emplace_back(file);
        }
        return print_mutation_result(downloads.install_offline(files, yes), "install-offline", files);
    }

    fedora_pm::system::RepositoryManager repos;
    if (command == "repo-list" && argc == 2) return print_result(repos.list());
    if (command == "repo-info" && argc == 3) return print_result(repos.info(argv[2]));
    if (command == "repo-add" && (argc == 3 || argc == 4)) {
        if (argc == 3)
            return print_mutation_result(repos.add(argv[2]), "repo-add", {argv[2]});
        return print_mutation_result(repos.add(argv[2], argv[3]), "repo-add", {argv[2], argv[3]});
    }
    if (command == "repo-refresh" && argc == 2) return print_mutation_result(repos.refresh(), "repo-refresh", {});
    if (command == "repo-remove" && argc == 3)
        return print_mutation_result(repos.remove(argv[2]), "repo-remove", {argv[2]});
    if ((command == "repo-enable" || command == "repo-disable") && argc == 3)
        return print_mutation_result(command == "repo-enable" ? repos.enable(argv[2]) : repos.disable(argv[2]), command, {argv[2]});

    fedora_pm::packages::FlatpakManager flatpak;
    if (command == "flatpak-search") {
        if (argc != 3) { std::cerr << "Usage: fedora-pm flatpak-search QUERY\n"; return 2; }
        return print_result(flatpak.search(argv[2]));
    }
    if (command == "flatpak-list" && argc == 2) return print_result(flatpak.list_installed());
    if (command == "flatpak-info" && argc == 3) return print_result(flatpak.info(argv[2]));
    if (command == "flatpak-update") {
        if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--yes")) {
            std::cerr << "Usage: fedora-pm flatpak-update [--yes]\n";
            return 2;
        }
        return print_mutation_result(flatpak.update(has_yes(argc, argv, 2)), "flatpak-update", {"all"});
    }
    if (command == "flatpak-setup-flathub" && argc == 2) return print_mutation_result(flatpak.setup_flathub(), "flatpak-setup-flathub", {"flathub"});
    if (command == "flatpak-install" || command == "flatpak-remove") {
        if (argc < 3 || argc > 4 || (argc == 4 && std::string(argv[3]) != "--yes")) {
            std::cerr << "Usage: fedora-pm " << command << " APP_ID [--yes]\n";
            return 2;
        }
        const bool yes = has_yes(argc, argv, 3);
        return print_mutation_result(command == "flatpak-install"
            ? flatpak.install(argv[2], yes)
            : flatpak.remove(argv[2], yes), command, {argv[2]});
    }

    if (command == "install" || command == "remove" || command == "update") {
        bool yes = false;
        std::vector<std::string> packages;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--yes") yes = true;
            else if (arg.starts_with("-")) {
                std::cerr << "Unknown " << command << " option: " << arg << '\\n';
                return 2;
            } else {
                packages.emplace_back(arg);
            }
        }
        if ((command == "install" || command == "remove") && packages.empty()) {
            std::cerr << "Usage: fedora-pm " << command << " [--yes] NAME...\\n";
            return 2;
        }
        if (command == "install")
            return print_mutation_result(pm.install(packages, yes), "install", packages);
        if (command == "remove")
            return print_mutation_result(pm.remove(packages, yes), "remove", packages);
        const auto items = packages.empty() ? std::vector<std::string>{"system"} : packages;
        return print_mutation_result(pm.update(packages, yes), "update", items);
    }
    usage();
    return 2;
}
