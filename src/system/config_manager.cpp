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
    static constexpr char hex[] = "0123456789abcdef";
    std::string out;
    for (unsigned char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                out += "\\u00";
                out.push_back(hex[(c >> 4) & 0x0f]);
                out.push_back(hex[c & 0x0f]);
            } else {
                out.push_back(static_cast<char>(c));
            }
        }
    }
    return out;
}
class JsonValidator {
public:
    explicit JsonValidator(const std::string& input) : input_(input) {}
    bool valid_object() {
        skip_space();
        if (!parse_object()) return false;
        skip_space();
        return pos_ == input_.size();
    }
private:
    const std::string& input_;
    std::size_t pos_{0};

    void skip_space() {
        while (pos_ < input_.size() &&
               (input_[pos_] == ' ' || input_[pos_] == '\t' ||
                input_[pos_] == '\r' || input_[pos_] == '\n')) ++pos_;
    }
    bool consume(char expected) {
        skip_space();
        if (pos_ >= input_.size() || input_[pos_] != expected) return false;
        ++pos_;
        return true;
    }
    bool parse_string() {
        if (pos_ >= input_.size() || input_[pos_++] != '"') return false;
        while (pos_ < input_.size()) {
            const unsigned char c = static_cast<unsigned char>(input_[pos_++]);
            if (c == '"') return true;
            if (c < 0x20) return false;
            if (c != '\\') continue;
            if (pos_ >= input_.size()) return false;
            const char escape = input_[pos_++];
            if (escape == '"' || escape == '\\' || escape == '/' ||
                escape == 'b' || escape == 'f' || escape == 'n' ||
                escape == 'r' || escape == 't') continue;
            if (escape != 'u' || input_.size() - pos_ < 4) return false;
            for (int i = 0; i < 4; ++i) {
                const char h = input_[pos_++];
                if (!((h >= '0' && h <= '9') || (h >= 'a' && h <= 'f') ||
                      (h >= 'A' && h <= 'F'))) return false;
            }
        }
        return false;
    }
    bool parse_number() {
        const std::size_t start = pos_;
        if (pos_ < input_.size() && input_[pos_] == '-') ++pos_;
        if (pos_ >= input_.size()) return false;
        if (input_[pos_] == '0') {
            ++pos_;
            if (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') return false;
        } else {
            if (input_[pos_] < '1' || input_[pos_] > '9') return false;
            while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') ++pos_;
        }
        if (pos_ < input_.size() && input_[pos_] == '.') {
            ++pos_;
            const std::size_t digits = pos_;
            while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') ++pos_;
            if (digits == pos_) return false;
        }
        if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) ++pos_;
            const std::size_t digits = pos_;
            while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') ++pos_;
            if (digits == pos_) return false;
        }
        return pos_ > start;
    }
    bool parse_array() {
        if (!consume('[')) return false;
        skip_space();
        if (pos_ < input_.size() && input_[pos_] == ']') { ++pos_; return true; }
        while (true) {
            if (!parse_value()) return false;
            skip_space();
            if (pos_ < input_.size() && input_[pos_] == ']') { ++pos_; return true; }
            if (!consume(',')) return false;
        }
    }
    bool parse_object() {
        if (!consume('{')) return false;
        skip_space();
        if (pos_ < input_.size() && input_[pos_] == '}') { ++pos_; return true; }
        while (true) {
            skip_space();
            if (!parse_string() || !consume(':') || !parse_value()) return false;
            skip_space();
            if (pos_ < input_.size() && input_[pos_] == '}') { ++pos_; return true; }
            if (!consume(',')) return false;
        }
    }
    bool parse_value() {
        skip_space();
        if (pos_ >= input_.size()) return false;
        if (input_[pos_] == '"') return parse_string();
        if (input_[pos_] == '{') return parse_object();
        if (input_[pos_] == '[') return parse_array();
        if (input_[pos_] == '-' || (input_[pos_] >= '0' && input_[pos_] <= '9')) return parse_number();
        for (const char* literal : {"true", "false", "null"}) {
            const std::string token(literal);
            if (input_.compare(pos_, token.size(), token) == 0) { pos_ += token.size(); return true; }
        }
        return false;
    }
};
bool is_valid_json_object(const std::string& data) {
    return JsonValidator(data).valid_object();
}
bool has_valid_config_types(const std::string& data) {
    for (const std::string key : {"auto_clean", "parallel_downloads", "fastest_mirror", "color_output"}) {
        const std::regex field(std::string("\\\"") + key + R"re("\\s*:\\s*)re");
        std::smatch match;
        if (!std::regex_search(data, match, field)) continue;
        const std::size_t value_pos = static_cast<std::size_t>(match.position() + match.length());
        if (data.compare(value_pos, 4, "true") == 0 || data.compare(value_pos, 5, "false") == 0) {
            const std::size_t len = data.compare(value_pos, 4, "true") == 0 ? 4 : 5;
            if (value_pos + len < data.size() &&
                data[value_pos + len] != ',' && data[value_pos + len] != '}' &&
                data[value_pos + len] != ' ' && data[value_pos + len] != '\t' &&
                data[value_pos + len] != '\r' && data[value_pos + len] != '\n') return false;
        } else {
            return false;
        }
    }
    const std::regex history_field(R"re("history_file"\\s*:\\s*)re");
    std::smatch history_match;
    if (std::regex_search(data, history_match, history_field)) {
        const std::size_t value_pos = static_cast<std::size_t>(history_match.position() + history_match.length());
        if (value_pos >= data.size() || data[value_pos] != '"') return false;
    }
    return true;
}
bool parse_bool(const std::string& data, const std::string& key, bool fallback) {
    const std::regex pattern(std::string("\"") + key + R"re("\s*:\s*(true|false))re");
    std::smatch match;
    if (!std::regex_search(data, match, pattern)) return fallback;
    return match[1].str() == "true";
}
std::string parse_string(const std::string& data, const std::string& key, const std::string& fallback) {
    const std::regex pattern(std::string("\"") + key + R"re("\s*:\s*"((?:[^"\\]|\\.)*)")re");
    std::smatch match;
    if (!std::regex_search(data, match, pattern)) return fallback;
    const std::string value = match[1].str();
    std::string result;
    const auto append_utf8 = [&result](unsigned int cp) {
        if (cp <= 0x7f) result.push_back(static_cast<char>(cp));
        else if (cp <= 0x7ff) {
            result.push_back(static_cast<char>(0xc0 | (cp >> 6)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
        } else {
            result.push_back(static_cast<char>(0xe0 | (cp >> 12)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
        }
    };
    const auto hex_value = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] != '\\' || i + 1 >= value.size()) {
            result.push_back(value[i]);
            continue;
        }
        const char escaped = value[++i];
        switch (escaped) {
        case '"': result.push_back('"'); break;
        case '\\': result.push_back('\\'); break;
        case '/': result.push_back('/'); break;
        case 'b': result.push_back('\b'); break;
        case 'f': result.push_back('\f'); break;
        case 'n': result.push_back('\n'); break;
        case 'r': result.push_back('\r'); break;
        case 't': result.push_back('\t'); break;
        case 'u': {
            if (i + 4 >= value.size()) { result += "\xef\xbf\xbd"; i = value.size(); break; }
            unsigned int cp = 0;
            bool valid = true;
            for (int digit = 0; digit < 4; ++digit) {
                const int hex = hex_value(value[i + 1 + digit]);
                if (hex < 0) { valid = false; break; }
                cp = (cp << 4) | static_cast<unsigned int>(hex);
            }
            if (!valid) { result += "\xef\xbf\xbd"; break; }
            i += 4;
            if (cp >= 0xd800 && cp <= 0xdbff && i + 6 < value.size() &&
                value[i + 1] == '\\' && value[i + 2] == 'u') {
                unsigned int low = 0;
                bool valid_low = true;
                for (int digit = 0; digit < 4; ++digit) {
                    const int hex = hex_value(value[i + 3 + digit]);
                    if (hex < 0) { valid_low = false; break; }
                    low = (low << 4) | static_cast<unsigned int>(hex);
                }
                if (valid_low && low >= 0xdc00 && low <= 0xdfff) {
                    const unsigned int combined = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                    result.push_back(static_cast<char>(0xf0 | (combined >> 18)));
                    result.push_back(static_cast<char>(0x80 | ((combined >> 12) & 0x3f)));
                    result.push_back(static_cast<char>(0x80 | ((combined >> 6) & 0x3f)));
                    result.push_back(static_cast<char>(0x80 | (combined & 0x3f)));
                    i += 6;
                    break;
                }
            }
            if (cp >= 0xd800 && cp <= 0xdfff) result += "\\xef\\xbf\\xbd";
            else append_utf8(cp);
            break;
        }
        default: result.push_back(escaped); break;
        }
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
    if (!is_valid_json_object(data))
        return {false, {}, "Config file is not valid JSON; leaving configuration unchanged: " + path.string()};
    if (!has_valid_config_types(data))
        return {false, {}, "Config file has invalid value types; leaving configuration unchanged: " + path.string()};
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
    // Fedora/Linux rename replaces an existing destination atomically. Do not
    // remove the old config as a fallback: if the second rename then fails,
    // that would destroy the user's last valid configuration.
    std::filesystem::rename(temp, path, ec);
    if (ec) {
        std::error_code cleanup_ec;
        std::filesystem::remove(temp, cleanup_ec);
        return {false, {}, "Could not atomically replace config file: " + ec.message()};
    }
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
PackageResult ConfigManager::get(const std::string& key) {
    auto loaded = load();
    if (!loaded.success) return loaded;
    if (key == "auto_clean") return {true, config_.auto_clean ? "true\n" : "false\n", {}};
    if (key == "parallel_downloads") return {true, config_.parallel_downloads ? "true\n" : "false\n", {}};
    if (key == "fastest_mirror") return {true, config_.fastest_mirror ? "true\n" : "false\n", {}};
    if (key == "color_output") return {true, config_.color_output ? "true\n" : "false\n", {}};
    if (key == "history_file") return {true, config_.history_file.string() + "\n", {}};
    return {false, {}, "Unknown config key: " + key};
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
