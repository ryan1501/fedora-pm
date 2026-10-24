#include "system/config_manager.hpp"
#include "core/validation.hpp"

#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
std::filesystem::path home_directory() {
    if (const char* home = std::getenv("HOME"); home && *home) return home;
    return std::filesystem::current_path();
}
std::string json_escape(const std::string& value) {
    std::string out;
    for (char c : value) {
        if (c == '\\' || c == '"') out.push_back('\\');
        if (c == '\n') { out += "\\n"; continue; }
        if (c == '\r') { out += "\\r"; continue; }
        out.push_back(c);
    }
    return out;
}
bool parse_bool(const std::string& data, const std::string& key, bool fallback) {
    const std::regex pattern("\\"" + key + "\\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (!std::regex_search(data, match, pattern)) return fallback;
    return match[1].str() == "true";
}
std::string parse_string(const std::string& data, const std::string& key, const std::string& fallback) {
    const std::regex pattern("\\"" + key + "\\"\\s*:\\s*\\"((?:[^\\"\\\\]|\\\\.)*)\\"");
    std::smatch match;
    if (!std::regex_search(data, match, pattern)) return fallback;
    std::string value = match[1].str(), result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            if (next == 'n') result.push_back('\n');
            else if (next == 'r') result.push_back('\r');
            else result.push_back(next);
        } else result.push_back(value[i]);
    }
    return result;
}
}
ConfigManager::ConfigManager(std::filesystem::path config_dir) {
    config_.config_dir = config_dir.empty() ? home_directory() / ".fedora-pm" : std::move(config_dir);
    config_.history_file = config_.config_dir / "history.json";
}
PackageResult ConfigManager::load() {
    std::error_code ec;
    std::filesystem::create_directories(config_.config_dir, ec);
    if (ec) return {false, {}, "Could not create config directory: " + ec.message()};
    const auto path = config_.config_dir / "config.json";
    if (!std::filesystem::exists(path, ec)) return save();
    return parse_file(path);
}
PackageResult ConfigManager::parse_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) return {false, {}, "Could not open config file: " + path.string()};
    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string data = buffer.str();
    if (data.find('{') == std::string::npos || data.find('}') == std::string::npos)
        return {false, {}, "Config file is not a valid JSON object: " + path.string()};
    config_.auto_clean = parse_bool(data, "auto_clean", false);
    config_.parallel_downloads = parse_bool(data, "parallel_downloads", true);
    config_.fastest_mirror = parse_bool(data, "fastest_mirror", true);
    config_.color_output = parse_bool(data, "color_output", true);
    const auto history = parse_string(data, "history_file", (config_.config_dir / "history.json").string());
    config_.history_file = history.empty() ? config_.config_dir / "history.json" : std::filesystem::path(history);
    return {true, "Loaded configuration from " + path.string() + "\n", {}};
}
PackageResult ConfigManager::save() const {
    std::error_code ec;
    std::filesystem::create_directories(config_.config_dir, ec);
    if (ec) return {false, {}, "Could not create config directory: " + ec.message()};
    const auto path = config_.config_dir / "config.json";
    const auto temp = config_.config_dir / "config.json.tmp";
    std::ofstream output(temp, std::ios::trunc);
    if (!output) return {false, {}, "Could not write config file: " + temp.string()};
    output << "{\n"
           << "  \"auto_clean\": " << (config_.auto_clean ? "true" : "false") << ",\n"
           << "  \"parallel_downloads\": " << (config_.parallel_downloads ? "true" : "false") << ",\n"
           << "  \"fastest_mirror\": " << (config_.fastest_mirror ? "true" : "false") << ",\n"
           << "  \"color_output\": " << (config_.color_output ? "true" : "false") << ",\n"
           << "  \"history_file\": \"" << json_escape(config_.history_file.string()) << "\"\n"
           << "}\n";
    output.close();
    if (!output) return {false, {}, "Failed writing configuration data."};
    std::filesystem::rename(temp, path, ec);
    if (ec) {
        std::filesystem::remove(path, ec);
        ec.clear();
        std::filesystem::rename(temp, path, ec);
    }
    if (ec) return {false, {}, "Could not atomically replace config file: " + ec.message()};
    return {true, "Saved configuration to " + path.string() + "\n", {}};
}
PackageResult ConfigManager::show() {
    auto loaded = load();
    if (!loaded.success) return loaded;
    std::ostringstream out;
    out << "Configuration file: " << (config_.config_dir / "config.json").string() << '\n'
        << "auto_clean: " << (config_.auto_clean ? "true" : "false") << '\n'
        << "parallel_downloads: " << (config_.parallel_downloads ? "true" : "false") << '\n'
        << "fastest_mirror: " << (config_.fastest_mirror ? "true" : "false") << '\n'
        << "color_output: " << (config_.color_output ? "true" : "false") << '\n'
        << "history_file: " << config_.history_file.string() << '\n';
    return {true, out.str(), {}};
}
PackageResult ConfigManager::set(const std::string& key, const std::string& value) {
    auto loaded = load();
    if (!loaded.success) return loaded;
    if (key == "history_file") {
        if (fedora_pm::core::trim(value).empty()) return {false, {}, "history_file must not be empty."};
        config_.history_file = value;
    } else {
        if (value != "true" && value != "false") return {false, {}, "Boolean options must be set to true or false."};
        const bool enabled = value == "true";
        if (key == "auto_clean") config_.auto_clean = enabled;
        else if (key == "parallel_downloads") config_.parallel_downloads = enabled;
        else if (key == "fastest_mirror") config_.fastest_mirror = enabled;
        else if (key == "color_output") config_.color_output = enabled;
        else return {false, {}, "Unknown config key: " + key};
    }
    return save();
}
}
