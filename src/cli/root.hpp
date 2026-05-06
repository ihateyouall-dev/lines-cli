#include "CLI/CLI11.hpp"
#include "cli/docs/docs.hpp"
#include "cli/tasks/tasks.hpp"

class Root : public CLI::App {
    Tasks _tasks;
    Docs _docs;

  public:
    Root();
};
