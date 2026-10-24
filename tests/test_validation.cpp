#include "core/validation.hpp"
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
}
