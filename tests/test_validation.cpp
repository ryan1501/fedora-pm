#include "core/validation.hpp"
#include "package_manager/package_manager.hpp"
#include "package_manager/download_manager.hpp"
#include "package_manager/changelog_manager.hpp"
#include "package_manager/export_manager.hpp"
#include "package_manager/flatpak_manager.hpp"
#include "system/disk_space_manager.hpp"
#include "system/security_manager.hpp"
#include "system/config_manager.hpp"
#include "system/driver_manager.hpp"
#include "system/rollback_manager.hpp"
#include "system/repository_manager.hpp"
#include "system/kernel_manager.hpp"
#include "system/cachyos_manager.hpp"
#include "system/history_manager.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    using fedora_pm::core::is_valid_package_name;
    using fedora_pm::core::trim;
    assert(is_valid_package_name("firefox"));
    assert(is_valid_package_name("mesa-vulkan-drivers"));
    assert(is_valid_package_name("python3.12"));
    assert(!is_valid_package_name(""));
    assert(!is_valid_package_name("firefox; rm -rf /"));
    assert(!is_valid_package_name("foo/bar"));
    assert(trim("  fedora  ") == "fedora");
    assert(trim(" \t\n ") == "");

    const auto no_packages = fedora_pm::packages::DownloadManager{}.download({});
    assert(!no_packages.success);
    const auto unsafe_package = fedora_pm::packages::DownloadManager{}.download({"bad;name"});
    assert(!unsafe_package.success);

    const fedora_pm::system::DiskSpaceManager disk;
    assert(!disk.top_packages(0).success);
    assert(!disk.top_packages(1001).success);
    assert(!disk.package_size("not/a-package").success);

    const fedora_pm::packages::ChangelogManager changelog;
    assert(!changelog.show("bad/name").success);
    assert(!changelog.show_recent("firefox", 0).success);
    assert(!changelog.show_recent("firefox", 501).success);
    assert(!changelog.whats_new(0).success);
    assert(!changelog.whats_new(101).success);
    assert(!changelog.compare("bad/name", "1", "2").success);
    assert(!changelog.compare("firefox", "", "2").success);

    std::error_code cleanup_error;
    const auto invalid_backup = std::filesystem::temp_directory_path() / "fedora-pm-invalid-backup.txt";
    { std::ofstream backup(invalid_backup); backup << "# flatpak:\nflatpak:org..invalid\n"; }
    assert(!fedora_pm::packages::ExportManager{}.import_from_file(invalid_backup.string(), true).success);
    std::filesystem::remove(invalid_backup, cleanup_error);

    const fedora_pm::system::SecurityManager security;
    assert(!security.list("arbitrary-command").success);
    assert(!security.advisory_info("bad;command").success);
    assert(!security.cve_check("CVE-2026-1;rm").success);

    const auto test_config_dir = std::filesystem::temp_directory_path() / "fedora-pm-cpp-config-test";
    std::filesystem::remove_all(test_config_dir, cleanup_error);
    fedora_pm::system::ConfigManager config(test_config_dir);
    assert(config.load().success);
    assert(config.set("auto_clean", "true").success);
    assert(config.get("auto_clean").output == "true\n");
    assert(!config.set("auto_clean", "yes").success);
    assert(!config.set("unknown_key", "true").success);

    const std::string special_history_path = (test_config_dir / "path\\with\"quotes\tand-café.json").string();
    assert(config.set("history_file", special_history_path).success);
    fedora_pm::system::ConfigManager reloaded_config(test_config_dir);
    assert(reloaded_config.load().success);
    assert(reloaded_config.get("history_file").output == special_history_path + "\n");
    std::filesystem::remove_all(test_config_dir, cleanup_error);

    const fedora_pm::system::RollbackManager rollback;
    assert(!rollback.rollback("0").success);
    assert(!rollback.rollback("1;rm").success);
    const fedora_pm::system::DriverManager drivers;
    assert(!drivers.install_nvidia(std::string("bad;version")).success);

    const fedora_pm::packages::FlatpakManager flatpak;
    assert(!flatpak.install("bad/name").success);
    assert(!flatpak.info("org..example.App").success);
    const fedora_pm::system::RepositoryManager repos;
    assert(!repos.add("file:///etc/passwd").success);
    assert(!repos.add("https://example.org/repo repo").success);

    const fedora_pm::system::KernelManager kernels;
    assert(!kernels.install(std::string("bad;version")).success);
    assert(!kernels.remove({}, false).success);
    assert(!kernels.remove_old(0).success);

    const fedora_pm::system::CachyOSManager cachyos;
    assert(!cachyos.enable_repository("unknown").success);
    assert(!cachyos.disable_repository("unknown").success);
    assert(!cachyos.install_kernel("unknown").success);
    assert(!cachyos.install_kernel("default", "unknown").success);

    const fedora_pm::packages::PackageManager packages;
    assert(!packages.list_available("bad;pattern").success);
    assert(!packages.list_installed("bad;pattern").success);
    assert(!packages.group_info("").success);
    assert(!packages.install_group("\n").success);
    assert(!packages.remove_group(" \r ").success);

    const auto history_path = std::filesystem::temp_directory_path() / "fedora-pm-cpp-history-test.json";
    std::filesystem::remove(history_path, cleanup_error);
    const fedora_pm::system::HistoryManager history(history_path);
    assert(history.log("install", {"firefox", "vim"}).success);
    assert(history.log("remove", {"vim"}).success);
    assert(history.log("install", {"café", "quoted\"package", "emoji-🚀"}).success);
    const auto recent = history.print_recent(3);
    assert(recent.success);
    assert(recent.output.find("install: firefox, vim") != std::string::npos);
    assert(recent.output.find("remove: vim") != std::string::npos);
    assert(recent.output.find("café") != std::string::npos);
    assert(recent.output.find("quoted\"package") != std::string::npos);
    {
        std::ofstream legacy(history_path, std::ios::trunc);
        legacy << R"json([{"action":"install","items":["caf\u00e9","rocket \uD83D\uDE80"],"timestamp":"2026-10-24T12:00:00-05:00"}])json";
    }
    const auto unicode_history = history.print_recent(1);
    assert(unicode_history.success);
    assert(unicode_history.output.find("café") != std::string::npos);
    assert(unicode_history.output.find("rocket 🚀") != std::string::npos);
    std::filesystem::remove(history_path, cleanup_error);
}
