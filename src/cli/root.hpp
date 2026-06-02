#include "CLI/CLI.hpp"
#include "cli/config/config.hpp"
#include "cli/docs/docs.hpp"
#include "cli/tasks/tasks.hpp"

namespace Lines::CLI {
class Root : public ::CLI::App {
    TasksCmd _tasks;
    DocsCmd _docs;
    ConfigCmd _config;

  public:
    Root();
};
} // namespace Lines::CLI
