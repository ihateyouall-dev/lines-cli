#include "cli/config/config.hpp"

#include "CLI/CLI.hpp"
#include "client-utils/parsers.hpp"
#include "client-utils/utils.hpp"

#include <format>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
template <typename Tp> void assign_to(void *ptr, Tp val) {
    *static_cast<Tp *>(ptr) = val;
}
} // namespace

Lines::CLI::ConfigCmd::ConfigCmd() {
    _storage.load_from_file();
    _cfg = &_storage.config();
}

void Lines::CLI::ConfigCmd::initially_register_keys() {
    register_key(
        "alwaysForce",
        {.ptr = &_cfg->always_force,
         .description = "Always force actions that require confirmation",
         .default_value = "false",
         .type = "BOOLEAN"});
    register_key("cli.colorize", {.ptr = &_cfg->cli_colorize,
                                  .description = "Use ANSI colors in stdout",
                                  .default_value = "true",
                                  .type = "BOOLEAN"});
    register_key("cli.useUnicode",
                 {.ptr = &_cfg->cli_use_unicode,
                  .description = "Use unicode symbols in stdout",
                  .default_value = "true",
                  .type = "BOOLEAN"});
}

void Lines::CLI::ConfigCmd::init(::CLI::App &app) {
    auto *config =
        app.add_subcommand("config", "Work with configuration")->alias("cfg");

    config->add_option("Key", _key, "Key in config")->required();
    config->add_option("Value", _val, "Give a new value to the key");

    initially_register_keys();

    config->callback([this]() -> void {
        if (!_keys.contains(_key)) {
            throw ::CLI::ValidationError(ClientUtils::error_str(
                "ERROR:", std::format("Key \"{}\" does not exist", _key)));
        }
        // Printing info about key if second positional (value) is not given
        if (!_val) {
            print_key_info(_key);
            return;
        }

        try {
            assign_value_to_key(_key, *_val);
            _dirty = true;
        } catch (const std::invalid_argument &e) {
            throw ::CLI::ValidationError(e.what());
        }
    });
}

void Lines::CLI::ConfigCmd::register_key(const std::string &key,
                                         const ConfigKeyInfo &info) {
    _keys[key] = info;
}

void Lines::CLI::ConfigCmd::print_key_value(const std::string &key,
                                            const ConfigKeyInfo &key_info) {
    if (key_info.type == "BOOLEAN") {
        std::cout << std::boolalpha << get_key_value<bool>(key) << '\n';
    }
}

void Lines::CLI::ConfigCmd::print_key_info(const std::string &key) {
    const auto &key_info = _keys[key];

    std::cout << std::format("Key: {}\n{}\nValue type: {}\nValue: ", key,
                             key_info.description, key_info.type);

    print_key_value(key, key_info);

    std::cout << std::format("Default value: {}\n", key_info.default_value);
    if (!key_info.possible_values.empty()) {
        std::cout << key_info.possible_values << '\n';
    }
}

void Lines::CLI::ConfigCmd::assign_value_to_key(
    const std::string &key, // NOLINT
    const std::string &value) {
    const auto &key_info = get_key_info(key);
    auto key_type = key_info.type;

    if (key_type == "BOOLEAN") {
        assign_to(key_info.ptr, ClientUtils::Parsers::parse<bool>(value));
    }
}

auto Lines::CLI::ConfigCmd::get_key_info(const std::string &key)
    -> const ConfigKeyInfo & {
    return _keys[key];
}

void Lines::CLI::ConfigCmd::save() {
    if (_dirty) {
        _storage.save_to_file();
    }
}

auto Lines::CLI::ConfigCmd::config() -> ClientUtils::Config { return *_cfg; }
