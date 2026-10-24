#pragma once

#include "package_manager/package_manager.hpp"
#include <string>

namespace fedora_pm::system {
class RollbackManager {
public:
    fedora_pm::packages::PackageResult list_history() const;
    fedora_pm::packages::PackageResult rollback(std::string transaction_id, bool assume_yes = false) const;
    fedora_pm::packages::PackageResult rollback_last(bool assume_yes = false) const;
};
}
