#include "CLI/CLI.hpp"
#include "cli/docs/docs.hpp"
#include "cli/tasks/tasks.hpp"

namespace Lines::CLI {
class Root : public ::CLI::App {
    Tasks _tasks;
    Docs _docs;

  public:
    Root();
};
} // namespace Lines::CLI
