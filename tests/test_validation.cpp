#include "core/validation.hpp"
#include "package_manager/download_manager.hpp"
#include "package_manager/changelog_manager.hpp"
#include "system/disk_space_manager.hpp"
#include "system/security_manager.hpp"
#include <cassert>

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

    const fedora_pm::system::SecurityManager security;
    assert(!security.list("arbitrary-command").success);
    assert(!security.advisory_info("bad;command").success);
    assert(!security.cve_check("CVE-2026-1;rm").success);
}
