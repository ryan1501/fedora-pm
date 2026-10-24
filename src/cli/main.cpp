#include "core/validation.hpp"
#include "package_manager/package_manager.hpp"
#include "system/system_info.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace {
void usage() {
    std::cout << "fedora-pm (C++20 migration in progress)\n"
              << "Usage: fedora-pm <command> [arguments]\n"
              << "Commands: info, search QUERY, package-info NAME, list, install [--yes] NAME..., remove [--yes] NAME..., update [--yes], help\n";
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
    fedora_pm::packages::PackageResult result;
    if (command == "search" && argc >= 3) result = pm.search(argv[2]);
    else if (command == "package-info" && argc >= 3) result = pm.info(argv[2]);
    else if (command == "list") result = pm.list_installed();
    else if (command == "install" || command == "remove" || command == "update") {
        int i = 2; bool yes = false;
        if (i < argc && std::string(argv[i]) == "--yes") { yes = true; ++i; }
        std::vector<std::string> packages;
        for (; i < argc; ++i) packages.emplace_back(argv[i]);
        if (command == "install") result = pm.install(packages, yes);
        else if (command == "remove") result = pm.remove(packages, yes);
        else if (!packages.empty()) { std::cerr << "update takes no package arguments\n"; return 2; }
        else result = pm.update(yes);
    } else { usage(); return 2; }
    if (!result.output.empty()) std::cout << result.output;
    if (!result.error.empty()) std::cerr << result.error << (result.error.back() == '\n' ? "" : "\n");
    return result.success ? 0 : 1;
}
