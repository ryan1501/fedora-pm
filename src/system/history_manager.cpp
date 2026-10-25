#include "system/history_manager.hpp"
#include "system/config_manager.hpp"
#include "core/validation.hpp"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <system_error>
#include <vector>

namespace fedora_pm::system {
using fedora_pm::packages::PackageResult;
namespace {
std::filesystem::path default_history_path() {
    if (const char* home = std::getenv("HOME"); home && *home)
        return std::filesystem::path(home) / ".fedora-pm" / "history.json";
    return std::filesystem::current_path() / ".fedora-pm" / "history.json";
}
std::string escape_json(const std::string& value) {
    std::string out;
    for (unsigned char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    const char* hex = "0123456789abcdef";
                    out += "\\u00";
                    out.push_back(hex[(c >> 4) & 0x0f]);
                    out.push_back(hex[c & 0x0f]);
                } else out.push_back(static_cast<char>(c));
        }
    }
    return out;
}
std::string unescape_json(const std::string& value) {
    std::string out;
    const auto append_utf8 = [&out](unsigned int cp) {
        if (cp <= 0x7f) out.push_back(static_cast<char>(cp));
        else if (cp <= 0x7ff) {
            out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
        } else if (cp <= 0xffff) {
            out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
        } else if (cp <= 0x10ffff) {
            out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
        } else out += "\xef\xbf\xbd";
    };
    const auto hex4 = [&value](std::size_t pos, unsigned int& cp) {
        if (pos + 4 > value.size()) return false;
        cp = 0;
        for (std::size_t j = pos; j < pos + 4; ++j) {
            const char c = value[j];
            unsigned int digit;
            if (c >= '0' && c <= '9') digit = static_cast<unsigned int>(c - '0');
            else if (c >= 'a' && c <= 'f') digit = static_cast<unsigned int>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') digit = static_cast<unsigned int>(c - 'A' + 10);
            else return false;
            cp = (cp << 4) | digit;
        }
        return true;
    };
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] != '\\') { out.push_back(value[i]); continue; }
        if (++i >= value.size()) { out += "\xef\xbf\xbd"; break; }
        switch (value[i]) {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'u': {
                unsigned int cp = 0;
                if (!hex4(i + 1, cp)) { out += "\xef\xbf\xbd"; break; }
                i += 4;
                if (cp >= 0xd800 && cp <= 0xdbff && i + 6 < value.size() &&
                    value[i + 1] == '\\' && value[i + 2] == 'u') {
                    unsigned int low = 0;
                    if (hex4(i + 3, low) && low >= 0xdc00 && low <= 0xdfff) {
                        cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                        i += 6;
                    } else cp = 0xfffd;
                } else if (cp >= 0xd800 && cp <= 0xdfff) cp = 0xfffd;
                append_utf8(cp);
                break;
            }
            default: out += "\xef\xbf\xbd"; break;
        }
    }
    return out;
}
bool valid_json_document(const std::string& input) {
    class Parser {
    public:
        explicit Parser(const std::string& text) : text_(text) {}
        bool parse() {
            skip_space();
            if (!value(0)) return false;
            skip_space();
            return pos_ == text_.size();
        }
    private:
        const std::string& text_;
        std::size_t pos_ = 0;
        void skip_space() {
            while (pos_ < text_.size() &&
                   (text_[pos_] == ' ' || text_[pos_] == '\t' ||
                    text_[pos_] == '\r' || text_[pos_] == '\n')) ++pos_;
        }
        bool hex4() {
            if (pos_ + 4 > text_.size()) return false;
            for (int n = 0; n < 4; ++n) {
                const char c = text_[pos_++];
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                      (c >= 'A' && c <= 'F'))) return false;
            }
            return true;
        }
        bool string() {
            if (pos_ >= text_.size() || text_[pos_++] != '"') return false;
            while (pos_ < text_.size()) {
                const unsigned char c = static_cast<unsigned char>(text_[pos_++]);
                if (c == '"') return true;
                if (c < 0x20) return false;
                if (c != '\\') continue;
                if (pos_ >= text_.size()) return false;
                const char escaped = text_[pos_++];
                if (escaped == '"' || escaped == '\\' || escaped == '/' ||
                    escaped == 'b' || escaped == 'f' || escaped == 'n' ||
                    escaped == 'r' || escaped == 't') continue;
                if (escaped == 'u') {
                    if (!hex4()) return false;
                    continue;
                }
                return false;
            }
            return false;
        }
        bool number() {
            const std::size_t start = pos_;
            if (pos_ < text_.size() && text_[pos_] == '-') ++pos_;
            if (pos_ >= text_.size()) return false;
            if (text_[pos_] == '0') {
                ++pos_;
                if (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') return false;
            } else {
                if (text_[pos_] < '1' || text_[pos_] > '9') return false;
                while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_;
            }
            if (pos_ < text_.size() && text_[pos_] == '.') {
                ++pos_;
                const std::size_t digits = pos_;
                while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_;
                if (digits == pos_) return false;
            }
            if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
                ++pos_;
                if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) ++pos_;
                const std::size_t digits = pos_;
                while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_;
                if (digits == pos_) return false;
            }
            return pos_ > start;
        }
        bool literal(const char* word) {
            const std::string expected(word);
            if (text_.compare(pos_, expected.size(), expected) != 0) return false;
            pos_ += expected.size();
            return true;
        }
        bool value(unsigned int depth) {
            if (depth > 128) return false;
            skip_space();
            if (pos_ >= text_.size()) return false;
            if (text_[pos_] == '"') return string();
            if (text_[pos_] == '{') {
                ++pos_;
                skip_space();
                if (pos_ < text_.size() && text_[pos_] == '}') { ++pos_; return true; }
                while (true) {
                    skip_space();
                    if (!string()) return false;
                    skip_space();
                    if (pos_ >= text_.size() || text_[pos_++] != ':') return false;
                    if (!value(depth + 1)) return false;
                    skip_space();
                    if (pos_ >= text_.size()) return false;
                    if (text_[pos_] == '}') { ++pos_; return true; }
                    if (text_[pos_++] != ',') return false;
                }
            }
            if (text_[pos_] == '[') {
                ++pos_;
                skip_space();
                if (pos_ < text_.size() && text_[pos_] == ']') { ++pos_; return true; }
                while (true) {
                    if (!value(depth + 1)) return false;
                    skip_space();
                    if (pos_ >= text_.size()) return false;
                    if (text_[pos_] == ']') { ++pos_; return true; }
                    if (text_[pos_++] != ',') return false;
                }
            }
            if (text_[pos_] == '-' || (text_[pos_] >= '0' && text_[pos_] <= '9')) return number();
            if (literal("true") || literal("false") || literal("null")) return true;
            return false;
        }
    };
    return Parser(input).parse();
}

bool json_array_objects_only(const std::string& input) {
    if (input.size() < 2 || input.front() != '[' || input.back() != ']') return false;
    std::size_t pos = 1;
    const std::size_t end = input.size() - 1;
    const auto skip_space = [&]() {
        while (pos < end && (input[pos] == ' ' || input[pos] == '\t' ||
                             input[pos] == '\r' || input[pos] == '\n')) ++pos;
    };
    skip_space();
    if (pos == end) return true;
    while (pos < end) {
        if (input[pos] != '{') return false;
        int nesting = 0;
        bool in_string = false;
        bool escaped = false;
        for (; pos < end; ++pos) {
            const char c = input[pos];
            if (in_string) {
                if (escaped) escaped = false;
                else if (c == '\\') escaped = true;
                else if (c == '"') in_string = false;
                continue;
            }
            if (c == '"') { in_string = true; continue; }
            if (c == '{' || c == '[') ++nesting;
            else if (c == '}' || c == ']') {
                --nesting;
                if (nesting == 0) { ++pos; break; }
            }
        }
        if (nesting != 0 || in_string) return false;
        skip_space();
        if (pos == end) return true;
        if (input[pos] != ',') return false;
        ++pos;
        skip_space();
        if (pos == end) return false;
    }
    return false;
}

std::size_t matching_json_delimiter(const std::string& input, std::size_t start) {
    if (start >= input.size() || (input[start] != '{' && input[start] != '['))
        return std::string::npos;
    std::vector<char> delimiters;
    bool in_string = false;
    bool escaped = false;
    for (std::size_t i = start; i < input.size(); ++i) {
        const char c = input[i];
        if (in_string) {
            if (escaped) escaped = false;
            else if (c == '\\\\') escaped = true;
            else if (c == '"') in_string = false;
            continue;
        }
        if (c == '"') { in_string = true; continue; }
        if (c == '{' || c == '[') delimiters.push_back(c);
        else if (c == '}' || c == ']') {
            if (delimiters.empty()) return std::string::npos;
            const char expected = c == '}' ? '{' : '[';
            if (delimiters.back() != expected) return std::string::npos;
            delimiters.pop_back();
            if (delimiters.empty()) return i;
        }
    }
    return std::string::npos;
}

std::string timestamp_now() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%dT%H:%M:%S");
    char offset[8]{};
    if (std::strftime(offset, sizeof(offset), "%z", &local) > 0) {
        std::string tz(offset);
        if (tz.size() == 5) tz.insert(3, ":");
        out << tz;
    }
    return out.str();
}
}
HistoryManager::HistoryManager(std::filesystem::path path) {
    if (!path.empty()) {
        path_ = std::move(path);
        return;
    }
    ConfigManager config;
    const auto loaded = config.load();
    path_ = loaded.success ? config.config().history_file : default_history_path();
    if (path_.empty()) path_ = default_history_path();
}
std::pair<bool, std::vector<OperationHistoryEntry>> HistoryManager::read_all(std::string& error) const {
    std::error_code ec;
    const bool path_exists = std::filesystem::exists(path_, ec);
    if (ec) {
        error = "Could not inspect history path " + path_.string() + ": " + ec.message();
        return {false, {}};
    }
    if (!path_exists) return {true, {}};
    const bool regular_file = std::filesystem::is_regular_file(path_, ec);
    if (ec || !regular_file) {
        error = "History path is not a readable regular file: " + path_.string();
        if (ec) error += " (" + ec.message() + ")";
        return {false, {}};
    }
    std::ifstream input(path_);
    if (!input) { error = "Could not read history file: " + path_.string(); return {false, {}}; }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string data = buffer.str();
    if (fedora_pm::core::trim(data).empty()) return {true, {}};
    const std::string trimmed_data = fedora_pm::core::trim(data);
    if (!valid_json_document(trimmed_data) || !json_array_objects_only(trimmed_data)) {
        error = "History file is not a valid JSON array of operation entries: " + path_.string();
        return {false, {}};
    }
    const std::regex action_pattern(R"re("action"\s*:\s*"((?:[^"\\]|\\.)*)")re");
    const std::regex timestamp_pattern(R"re("timestamp"\s*:\s*"((?:[^"\\]|\\.)*)")re");
    const std::regex items_key_pattern(R"re("items"\s*:\s*\[)re");
    const std::regex string_pattern(R"re("((?:[^"\\]|\\.)*)")re");
    std::vector<std::string> objects;
    std::size_t pos = 1;
    const std::size_t array_end = data.size() - 1;
    while (pos < array_end) {
        while (pos < array_end && (data[pos] == ' ' || data[pos] == '\\t' ||
                                   data[pos] == '\\r' || data[pos] == '\\n' || data[pos] == ','))
            ++pos;
        if (pos >= array_end) break;
        const std::size_t object_end = matching_json_delimiter(data, pos);
        if (object_end == std::string::npos || data[pos] != '{') {
            error = "Could not parse an entry in history file: " + path_.string();
            return {false, {}};
        }
        objects.push_back(data.substr(pos, object_end - pos + 1));
        pos = object_end + 1;
    }
    std::vector<OperationHistoryEntry> entries;
    for (const std::string& object : objects) {
        std::smatch action_match, timestamp_match, items_match;
        if (!std::regex_search(object, action_match, action_pattern) ||
            !std::regex_search(object, timestamp_match, timestamp_pattern) ||
            !std::regex_search(object, items_match, items_key_pattern)) {
            error = "Could not parse an entry in history file: " + path_.string();
            return {false, {}};
        }
        OperationHistoryEntry entry;
        entry.action = unescape_json(action_match[1].str());
        entry.timestamp = unescape_json(timestamp_match[1].str());
        const std::size_t items_open = static_cast<std::size_t>(items_match.position() + items_match.length() - 1);
        const std::size_t items_close = matching_json_delimiter(object, items_open);
        if (items_close == std::string::npos) {
            error = "Could not parse items in history file: " + path_.string();
            return {false, {}};
        }
        const std::string item_data = object.substr(items_open + 1, items_close - items_open - 1);
        for (std::sregex_iterator item(item_data.begin(), item_data.end(), string_pattern), item_end; item != item_end; ++item)
            entry.items.push_back(unescape_json((*item)[1].str()));
        entries.push_back(std::move(entry));
    }
    return {true, std::move(entries)};
}
PackageResult HistoryManager::log(const std::string& action, const std::vector<std::string>& items) const {
    if (fedora_pm::core::trim(action).empty()) return {false, {}, "History action must not be empty."};
    std::string error;
    auto [ok, entries] = read_all(error);
    if (!ok) return {false, {}, error};
    entries.push_back({action, items, timestamp_now()});
    std::error_code ec;
    if (path_.has_parent_path()) std::filesystem::create_directories(path_.parent_path(), ec);
    if (ec) return {false, {}, "Could not create history directory: " + ec.message()};
    auto temp = path_;
    temp += ".tmp";
    std::ofstream output(temp, std::ios::trunc);
    if (!output) return {false, {}, "Could not write history file: " + temp.string()};
    output << "[\n";
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        output << "  {\n    \"action\": \"" << escape_json(entry.action) << "\",\n    \"items\": [";
        for (std::size_t j = 0; j < entry.items.size(); ++j) {
            if (j) output << ", ";
            output << "\"" << escape_json(entry.items[j]) << "\"";
        }
        output << "],\n    \"timestamp\": \"" << escape_json(entry.timestamp) << "\"\n  }";
        if (i + 1 < entries.size()) output << ',';
        output << '\n';
    }
    output << "]\n";
    output.close();
    if (!output) return {false, {}, "Failed while writing history data."};
    // On POSIX, rename replaces the destination atomically. Never delete the
    // existing history as a fallback: if replacement fails, preserve the
    // previous file and report the error instead of risking data loss.
    std::filesystem::rename(temp, path_, ec);
    if (ec) return {false, {}, "Could not replace history file: " + ec.message()};
    return {true, {}, {}};
}
PackageResult HistoryManager::print_recent(std::size_t limit) const {
    std::string error;
    auto [ok, entries] = read_all(error);
    if (!ok) return {false, {}, error};
    std::ostringstream out;
    if (entries.empty()) return {true, "No history found.\n", {}};
    const std::size_t start = entries.size() > limit ? entries.size() - limit : 0;
    out << "Recent package management history (last " << entries.size() - start << " entries):\n";
    for (std::size_t i = entries.size(); i > start; --i) {
        const auto& entry = entries[i - 1];
        out << "  [" << entry.timestamp << "] " << entry.action << ": ";
        for (std::size_t j = 0; j < entry.items.size(); ++j) {
            if (j) out << ", ";
            out << entry.items[j];
        }
        out << '\n';
    }
    return {true, out.str(), {}};
}
}
