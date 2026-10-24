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
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {
void usage() {
    std::cout << "fedora-pm (C++20 migration in progress)\n"
              << "Usage: fedora-pm <command> [arguments]\n"
              << "Commands:\n"
              << "  info                              Show OS and architecture\n"
              << "  doctor                            Run read-only system diagnostics\n"
              << "  search QUERY                      Search DNF packages\n"
              << "  package-info NAME                 Show package metadata\n"
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
              << "  security-check                    List available security updates\n"
              << "  security-list [SEVERITY]           List security/advisory updates\n"
              << "  security-info ADVISORY             Show advisory details\n"
              << "  cve-check CVE-YYYY-NNNN            Check updates for a CVE\n"
              << "  security-audit                    Summarize security/critical/important updates\n"
              << "  security-update [--yes]            Install security updates\n"
              << "  config show | get KEY | set KEY VALUE\n"
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
              << "  export FILE                       Export user-installed RPM package list\n"
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
              << "  repo-add URL                      Add a repository URL\n"
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
    if (argc < 2 || std::string(argv[1]) == "help" || std::string(argv[1]) == "--help") { usage(); return 0; }
    const std::string command = argv[1];
    if (command == "info") {
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
    if (command == "nvidia-remove" && (argc == 2 || argc == 3))
        return print_mutation_result(drivers.remove_nvidia(has_yes(argc, argv, 2)), "driver_remove_nvidia", {});
    if (command == "cuda-install" && (argc == 2 || argc == 3))
        return print_mutation_result(drivers.install_cuda(has_yes(argc, argv, 2)), "driver_install_cuda", {});
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
    if (command == "gaming-install" && (argc == 2 || argc == 3))
        return print_mutation_result(fedora_pm::system::GamingManager{}.install_meta(has_yes(argc, argv, 2)), "gaming_install", {"fedora-gaming-meta"});

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
            if (std::string(argv[i]) == "--yes") yes = true;
            else if (!version) version = argv[i];
            else { std::cerr << "Usage: fedora-pm kernel-install [VERSION] [--yes]\n"; return 2; }
        }
        return print_mutation_result(kernels.install(version, yes), "kernel_install", {version.value_or("kernel")});
    }
    if (command == "kernel-remove") {
        bool yes = false;
        std::vector<std::string> versions;
        for (int i = 2; i < argc; ++i) {
            if (std::string(argv[i]) == "--yes") yes = true;
            else versions.emplace_back(argv[i]);
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
        return print_mutation_result(kernels.remove_old(keep, yes), "kernel_remove_old", {std::to_string(keep)});
    }

    fedora_pm::system::RollbackManager rollback;
    if (command == "rollback-history" && argc == 2) return print_result(rollback.list_history());
    if (command == "rollback-last" && (argc == 2 || argc == 3))
        return print_mutation_result(rollback.rollback_last(has_yes(argc, argv, 2)), "rollback_last", {});
    if (command == "rollback" && (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) { std::cerr << "Usage: fedora-pm rollback ID [--yes]\n"; return 2; }
        return print_mutation_result(rollback.rollback(argv[2], yes), "rollback", {argv[2]});
    }

    fedora_pm::packages::PackageManager pm;
    if (command == "history") return print_result(fedora_pm::system::HistoryManager{}.print_recent(10));
    if (command == "rollback-history") return print_result(pm.history());
    if (command == "search" && argc >= 3) return print_result(pm.search(argv[2]));
    if (command == "package-info" && argc >= 3) return print_result(pm.info(argv[2]));
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
            else if (arg.starts_with("--")) { std::cerr << "Unknown list option: " << arg << '\\n'; return 2; }
            else if (pattern.empty()) pattern = arg;
            else { std::cerr << "Usage: fedora-pm list [--available|--installed|--all] [PATTERN]\\n"; return 2; }
        }
        if (all) {
            const auto first = pm.list_installed(pattern);
            const auto second = pm.list_available(pattern);
            return print_result({first.success && second.success, first.output + "\\n" + second.output, first.error + second.error});
        }
        if (available) return print_result(pm.list_available(pattern));
        return print_result(pm.list_installed(pattern));
    }
    if (command == "clean" && argc == 2) return print_mutation_result(pm.clean_cache(), "clean", {"dnf"});
    if (command == "group-info" && argc == 3) return print_result(pm.group_info(argv[2]));
    if (command == "group-install" && (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) { std::cerr << "Usage: fedora-pm group-install NAME [--yes]\\n"; return 2; }
        return print_mutation_result(pm.install_group(argv[2], yes), "group-install", {argv[2]});
    }
    if (command == "group-remove" && (argc == 3 || argc == 4)) {
        const bool yes = argc == 4 && std::string(argv[3]) == "--yes";
        if (argc == 4 && !yes) { std::cerr << "Usage: fedora-pm group-remove NAME [--yes]\\n"; return 2; }
        return print_mutation_result(pm.remove_group(argv[2], yes), "group-remove", {argv[2]});
    }
    if (command == "deps" && argc == 3) return print_result(pm.dependencies(argv[2]));
    if (command == "reverse-deps" && argc == 3) return print_result(pm.reverse_dependencies(argv[2]));
    if (command == "groups") return print_result(pm.installed_groups());

    fedora_pm::packages::ChangelogManager changelogs;
    if (command == "changelog" && argc == 3) return print_result(changelogs.show(argv[2]));
    if (command == "changelog-recent" && (argc == 3 || argc == 4)) {
        std::size_t count = 10;
        if (argc == 4) {
            const std::string value = argv[3];
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), count);
            if (ec != std::errc{} || ptr != value.data() + value.size()) {
                std::cerr << "COUNT must be a positive integer\n";
                return 2;
            }
        }
        return print_result(changelogs.show_recent(argv[2], count));
    }

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

    if (command == "config") {
        fedora_pm::system::ConfigManager config;
        if (argc == 3 && std::string(argv[2]) == "show") return print_result(config.show());
        if (argc == 4 && std::string(argv[2]) == "get") return print_result(config.get(argv[3]));
        if (argc == 5 && std::string(argv[2]) == "set") return print_result(config.set(argv[3], argv[4]));
        std::cerr << "Usage: fedora-pm config show | get KEY | set KEY VALUE\n";
        return 2;
    }

    if (command == "export" && argc == 3)
        return print_result(fedora_pm::packages::ExportManager{}.export_to_file(argv[2]));
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
    if (command == "diskspace") return print_result(disk.analyze());
    if (command == "orphans") return print_result(disk.orphaned_packages());
    if (command == "clean-orphans") return print_mutation_result(disk.remove_orphans(has_yes(argc, argv, 2)), "clean-orphans", {"autoremove"});
    if (command == "package-size" && argc == 3) return print_result(disk.package_size(argv[2]));
    if (command == "size") {
        std::size_t limit = 20;
        if (argc == 4 && std::string(argv[2]) == "--top") {
            const std::string value = argv[3];
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), limit);
            if (ec != std::errc{} || ptr != value.data() + value.size()) {
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
        int i = 2;
        bool with_deps = false;
        std::string destination = ".";
        while (i < argc) {
            const std::string arg = argv[i];
            if (arg == "--with-deps") { with_deps = true; ++i; }
            else if (arg == "--to" && i + 1 < argc) { destination = argv[i + 1]; i += 2; }
            else if (arg.starts_with("--")) { std::cerr << "Unknown download option: " << arg << '\n'; return 2; }
            else break;
        }
        std::vector<std::string> packages;
        for (; i < argc; ++i) packages.emplace_back(argv[i]);
        if (with_deps) return print_mutation_result(downloads.download_with_dependencies(packages, destination), "download-with-deps", packages);
        if (destination != ".") return print_mutation_result(downloads.download_to(packages, destination), "download", packages);
        return print_mutation_result(downloads.download(packages), "download", packages);
    }
    if (command == "install-offline") {
        int i = 2;
        bool yes = false;
        if (i < argc && std::string(argv[i]) == "--yes") { yes = true; ++i; }
        std::vector<std::string> files;
        for (; i < argc; ++i) files.emplace_back(argv[i]);
        return print_mutation_result(downloads.install_offline(files, yes), "install-offline", files);
    }

    fedora_pm::system::RepositoryManager repos;
    if (command == "repo-list") return print_result(repos.list());
    if (command == "repo-info" && argc == 3) return print_result(repos.info(argv[2]));
    if (command == "repo-add" && argc == 3) return print_mutation_result(repos.add(argv[2]), "repo-add", {argv[2]});
    if (command == "repo-refresh" && argc == 2) return print_mutation_result(repos.refresh(), "repo-refresh", {});
    if ((command == "repo-enable" || command == "repo-disable") && argc == 3)
        return print_mutation_result(command == "repo-enable" ? repos.enable(argv[2]) : repos.disable(argv[2]), command, {argv[2]});

    fedora_pm::packages::FlatpakManager flatpak;
    if (command == "flatpak-search" && argc >= 3) return print_result(flatpak.search(argv[2]));
    if (command == "flatpak-list") return print_result(flatpak.list_installed());
    if (command == "flatpak-info" && argc == 3) return print_result(flatpak.info(argv[2]));
    if (command == "flatpak-update" && (argc == 2 || argc == 3))
        return print_mutation_result(flatpak.update(has_yes(argc, argv, 2)), "flatpak-update", {"all"});
    if (command == "flatpak-setup-flathub" && argc == 2) return print_mutation_result(flatpak.setup_flathub(), "flatpak-setup-flathub", {"flathub"});
    if ((command == "flatpak-install" || command == "flatpak-remove") && argc >= 3) {
        const bool yes = has_yes(argc, argv, 3);
        return print_mutation_result(command == "flatpak-install"
            ? flatpak.install(argv[2], yes)
            : flatpak.remove(argv[2], yes), command, {argv[2]});
    }

    if (command == "install" || command == "remove" || command == "update") {
        int i = 2; bool yes = false;
        if (i < argc && std::string(argv[i]) == "--yes") { yes = true; ++i; }
        std::vector<std::string> packages;
        for (; i < argc; ++i) packages.emplace_back(argv[i]);
        if (command == "install") return print_mutation_result(pm.install(packages, yes), "install", packages);
        if (command == "remove") return print_mutation_result(pm.remove(packages, yes), "remove", packages);
        if (!packages.empty()) { std::cerr << "update takes no package arguments\n"; return 2; }
        return print_mutation_result(pm.update(yes), "update", packages.empty() ? std::vector<std::string>{"system"} : packages);
    }
    usage();
    return 2;
}
