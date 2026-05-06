#pragma once

#include <string>

namespace CLI {
class App;
}

class Docs {
    std::string manual;

  public:
    void init(CLI::App &app);
};
