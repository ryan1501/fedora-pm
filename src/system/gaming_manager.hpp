#pragma once

#include "package_manager/package_manager.hpp"

namespace fedora_pm::system {
class GamingManager {
public:
    fedora_pm::packages::PackageResult install_meta(bool assume_yes = false) const;
};
}
