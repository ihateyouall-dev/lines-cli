#pragma once

#include <filesystem>

namespace Lines::detail {
auto get_fs_home() -> std::filesystem::path;

auto get_fs_dotfile_storage() -> std::filesystem::path;
} // namespace Lines::detail
