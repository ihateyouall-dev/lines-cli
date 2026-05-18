#pragma once

#include <string>

namespace CLI {
class App;
}

namespace Lines::CLI {
class ConfigCmd {
    std::string _key;
    std::string _val;
};
} // namespace Lines::CLI
