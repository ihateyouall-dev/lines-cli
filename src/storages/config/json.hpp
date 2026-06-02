#pragma once

#include "client-utils/config.hpp"
#include "nlohmann/json_fwd.hpp"

#include <filesystem>
#include <utility>

namespace Lines {
namespace ConfigJSON {
auto from_json(const nlohmann::json &json) -> ClientUtils::Config;
auto to_json(const ClientUtils::Config &cfg) -> nlohmann::json;
} // namespace ConfigJSON
class ConfigJSONStorage {
    ClientUtils::Config _config;
    std::filesystem::path _file;

  public:
    explicit ConfigJSONStorage(std::filesystem::path path) : _file(std::move(path)) {}
    void load_from_json(const nlohmann::json &json);
    void load_from_file();

    [[nodiscard]] auto to_json() const -> nlohmann::json;
    void save_to_file() const;

    [[nodiscard]] auto config() const -> const ClientUtils::Config &;
    auto config() -> ClientUtils::Config &;
};
} // namespace Lines
