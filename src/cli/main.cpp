#include "core/validation.hpp"
#include "package_manager/package_manager.hpp"
#include "package_manager/flatpak_manager.hpp"
#include "package_manager/export_manager.hpp"
#include "system/system_info.hpp"
#include "system/repository_manager.hpp"
#include "system/kernel_manager.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace {
void usage() {
    std::cout << "fedora-pm (C++20 migration in progress)\n"
              << "Usage: fedora-pm <command> [arguments]\n"
              << "Commands:\n"
              << "  info                              Show OS and architecture\n"
              << "  search QUERY                      Search DNF packages\n"
              << "  package-info NAME                 Show package metadata\n"
              << "  list                              List installed RPM packages\n"
              << "  install [--yes] NAME...           Install RPM packages\n"
              << "  remove [--yes] NAME...            Remove RPM packages\n"
              << "  update [--yes]                    Update installed packages\n"
              << "  history                           Show DNF transaction history\n"
              << "  export FILE                       Export user-installed RPM package list\n"
              << "  import [--yes] FILE               Import and install a package list\n"
              << "  repo-list                         List DNF repositories\n"
              << "  repo-enable ID | repo-disable ID  Change repository state\n"
              << "  kernel-list                       Show running and installed kernels\n"
              << "  flatpak-search QUERY              Search Flatpak remotes\n"
              << "  flatpak-list                      List installed Flatpaks\n"
              << "  flatpak-install APP_ID [--yes]    Install a Flatpak\n"
              << "  flatpak-remove APP_ID [--yes]     Remove a Flatpak\n"
              << "  help                              Show this help\n";
}
int print_result(const fedora_pm::packages::PackageResult& result) {
    if (!result.output.empty()) std::cout << result.output;
    if (!result.error.empty()) std::cerr << result.error << (result.error.back() == '\n' ? "" : "\n");
    return result.success ? 0 : 1;
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

    fedora_pm::packages::PackageManager pm;
    if (command == "history") return print_result(pm.history());
    if (command == "search" && argc >= 3) return print_result(pm.search(argv[2]));
    if (command == "package-info" && argc >= 3) return print_result(pm.info(argv[2]));
    if (command == "list") return print_result(pm.list_installed());
    if (command == "export" && argc == 3)
        return print_result(fedora_pm::packages::ExportManager{}.export_to_file(argv[2]));
    if (command == "import") {
        const bool yes = has_yes(argc, argv, 2);
        const int path_index = yes ? 3 : 2;
        if (argc != path_index + 1) {
            std::cerr << "Usage: fedora-pm import [--yes] FILE\n";
            return 2;
        }
        return print_result(fedora_pm::packages::ExportManager{}.import_from_file(argv[path_index], yes));
    }

    if (command == "repo-list") return print_result(fedora_pm::system::RepositoryManager{}.list());
    if ((command == "repo-enable" || command == "repo-disable") && argc == 3) {
        fedora_pm::system::RepositoryManager repos;
        return print_result(command == "repo-enable" ? repos.enable(argv[2]) : repos.disable(argv[2]));
    }
    if (command == "kernel-list") {
        const auto kernels = fedora_pm::system::detect_kernels();
        std::cout << "Running kernel: " << kernels.running << "\nInstalled kernels:\n";
        for (const auto& kernel : kernels.installed) std::cout << "  " << kernel << "\n";
        return 0;
    }

    fedora_pm::packages::FlatpakManager flatpak;
    if (command == "flatpak-search" && argc >= 3) return print_result(flatpak.search(argv[2]));
    if (command == "flatpak-list") return print_result(flatpak.list_installed());
    if ((command == "flatpak-install" || command == "flatpak-remove") && argc >= 3) {
        const bool yes = has_yes(argc, argv, 3);
        return print_result(command == "flatpak-install"
            ? flatpak.install(argv[2], yes)
            : flatpak.remove(argv[2], yes));
    }

    if (command == "install" || command == "remove" || command == "update") {
        int i = 2; bool yes = false;
        if (i < argc && std::string(argv[i]) == "--yes") { yes = true; ++i; }
        std::vector<std::string> packages;
        for (; i < argc; ++i) packages.emplace_back(argv[i]);
        if (command == "install") return print_result(pm.install(packages, yes));
        if (command == "remove") return print_result(pm.remove(packages, yes));
        if (!packages.empty()) { std::cerr << "update takes no package arguments\n"; return 2; }
        return print_result(pm.update(yes));
    }
    usage();
    return 2;
}
