#pragma once

#include "package_manager/package_manager.hpp"
#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace fedora_pm::system {
struct OperationHistoryEntry {
    std::string action;
    std::vector<std::string> items;
    std::string timestamp;
};
class HistoryManager {
public:
    explicit HistoryManager(std::filesystem::path path = {});
    fedora_pm::packages::PackageResult log(const std::string& action,
                                            const std::vector<std::string>& items) const;
    fedora_pm::packages::PackageResult print_recent(std::size_t limit = 10) const;
    const std::filesystem::path& path() const { return path_; }
private:
    std::filesystem::path path_;
    std::pair<bool, std::vector<OperationHistoryEntry>> read_all(std::string& error) const;
};
}
