#include "CLI/CLI.hpp"
#include "cli/tasks/tasks.hpp"

class Root : public CLI::App {
    Tasks _tasks;

  public:
    Root();
};
