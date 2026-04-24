#include "storages/utils/filesystem.hpp"

auto Lines::detail::get_fs_home() -> std::filesystem::path {
#if defined(_WIN32)
    return std::getenv("USERPROFILE");
#else
    return std::getenv("HOME");
#endif
}

auto Lines::detail::get_fs_dotfile_storage() -> std::filesystem::path {
#if defined(_WIN32)
    return std::getenv("APPDATA");
#elif defined(__APPLE__)
    return get_fs_home() / "Library" / "Application Support";
#else
    return get_fs_home() / ".config";
#endif
}
