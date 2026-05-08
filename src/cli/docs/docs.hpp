#pragma once

#include <string>

namespace CLI {
class App;
}

namespace Lines::CLI {
class Docs {
    std::string manual;

  public:
    void init(::CLI::App &app);
};
} // namespace Lines::CLI
