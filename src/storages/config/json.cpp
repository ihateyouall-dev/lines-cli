#include "storages/config/json.hpp"

#include "nlohmann/json.hpp"

#include <fstream>

namespace {
template <typename Val>
void assign_from_json(Val &val, const nlohmann::json &json, std::string_view json_key) {
    val = json.value(json_key, val);
}
} // namespace

auto Lines::ConfigJSON::to_json(const ClientUtils::Config &cfg) -> nlohmann::json {
    nlohmann::json res;
    static constexpr int CFG_VER = 1;
    res["version"] = CFG_VER;

    res["alwaysForce"] = cfg.always_force;
    res["cli.colorize"] = cfg.cli_colorize;
    res["cli.useUnicode"] = cfg.cli_use_unicode;
    return res;
}

auto Lines::ConfigJSON::from_json(const nlohmann::json &json) -> ClientUtils::Config {
    ClientUtils::Config res;

    assign_from_json(res.always_force, json, "alwaysForce");
    assign_from_json(res.cli_colorize, json, "cli.colorize");
    assign_from_json(res.cli_use_unicode, json, "cli.useUnicode");
    return res;
}

auto Lines::ConfigJSONStorage::config() const -> const ClientUtils::Config & { return _config; }

void Lines::ConfigJSONStorage::load_from_json(const nlohmann::json &json) {
    _config = ConfigJSON::from_json(json);
}

auto Lines::ConfigJSONStorage::to_json() const -> nlohmann::json {
    return ConfigJSON::to_json(_config);
}

void Lines::ConfigJSONStorage::load_from_file() {
    std::ifstream fs(_file);

    if (!fs) {
        return;
    }

    nlohmann::json json;
    fs >> json;
    load_from_json(json);
}

auto Lines::ConfigJSONStorage::config() -> ClientUtils::Config & { return _config; }

void Lines::ConfigJSONStorage::save_to_file() const {
    std::filesystem::create_directories(_file.parent_path());
    std::ofstream fs(_file);

    if (!fs) {
        throw std::runtime_error("Lines::ConfigJSONStorage::save_to_file: cannot open file: " +
                                 _file.string());
    }

    fs << to_json().dump(4);
}
