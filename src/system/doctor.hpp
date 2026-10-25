#pragma once

#include "package_manager/package_manager.hpp"

namespace fedora_pm::system {
class Doctor {
public:
    fedora_pm::packages::PackageResult run_checks() const;
};
}
