#pragma once

#include "client-utils/config.hpp"
#include "client-utils/filesystem.hpp"
#include "storages/config/json.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace CLI {
class App;
}

namespace Lines::CLI {
class ConfigCmd {
    std::string _key;
    std::optional<std::string> _val;
    ClientUtils::Config *_cfg;
    ConfigJSONStorage _storage{ClientUtils::get_fs_home() / ".lines.d" / "config.json"};
    bool _dirty = false;

    struct ConfigKeyInfo {
        void *ptr;
        std::string description;
        std::string default_value;
        std::string type;
        std::string possible_values;
    };

    std::unordered_map<std::string, ConfigKeyInfo> _keys;

    void register_key(const std::string &key, const ConfigKeyInfo &info);
    void initially_register_keys();
    void print_key_value(const std::string &key, const ConfigKeyInfo &key_info);

    void print_key_info(const std::string &key);
    void assign_value_to_key(const std::string &key, const std::string &value);
    auto get_key_info(const std::string &key) -> const ConfigKeyInfo &;
    template <typename Tp> auto get_key_value(const std::string &key) {
        auto *key_ptr = get_key_info(key).ptr;
        return *static_cast<Tp *>(key_ptr);
    }

  public:
    ConfigCmd();

    void init(::CLI::App &app);
    auto config() -> ClientUtils::Config;
    void save();
};
} // namespace Lines::CLI
