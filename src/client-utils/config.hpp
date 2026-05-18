#pragma once

#include "client-utils/filesystem.hpp"

namespace Lines::ClientUtils {
struct Config {
    bool always_force = false;
    bool cli_use_unicode = true;
    bool cli_colorize = true;

    std::filesystem::path storage_data_path = get_fs_home() / ".lines.d" / "data";
};
} // namespace Lines::ClientUtils
