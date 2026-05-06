#pragma once

#include <string>

namespace Lines::ClientUtils::Colors {
constexpr auto red = "\x1b[31m";
constexpr auto green = "\x1b[32m";
constexpr auto yellow = "\x1b[33m";
constexpr auto blue = "\x1b[34m";

constexpr auto reset = "\x1b[0m";

constexpr auto colorize(const std::string &str, const std::string &color) -> std::string {
    return color + str + reset;
}
} // namespace Lines::ClientUtils::Colors
