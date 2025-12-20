#pragma once

#include <CLI11/CLI11.hpp>
#include <latertasks.hpp>

namespace Later {
class CLIApplication final {
    CLI::App main_app;
    CLI::App *tasks_app = nullptr;
    CLI::App *tasks_add = nullptr;
    CLI::App *tasks_list = nullptr;
    CLI::App *tasks_delete = nullptr;
    CLI::App *tasks_set = nullptr;
    Later::Tasks tasks;
    struct {
        std::optional<std::string> title;
        std::optional<std::string> description;
        std::optional<std::string> category;
    } task_args;
    struct Filter {
        bool completed = false;
        bool uncompleted = false;
        std::string category;

        void validate() {
            if (completed && uncompleted) {
                completed = false;
                uncompleted = false;
            }
        }

        [[nodiscard]] auto check(const Task &task) const -> bool {
            if (completed && !task.completed()) {
                return false;
            }
            if (uncompleted && task.completed()) {
                return false;
            }
            if (!category.empty() && category != task.category()) {
                return false;
            }
            return true;
        }
    } filter;
    int _argc;
    char **_argv;

    std::size_t task_number = 0;
    bool task_complete = false;
    bool task_uncomplete = false;

    std::filesystem::path tasks_config_path = "./latertasks.json";

    static auto get_executable_path() -> std::filesystem::path;

    static void print_version();

    static void print_task(const Task &task, size_t number);

    static void print_tasklist(const TaskList &tasklist, const Filter &filter);

    void attach_task_args(CLI::App *app);

    void setup_tasks();

    void setup_tasks_add();

    void setup_tasks_list();

    void setup_tasks_delete();

    void setup_tasks_set();

    static void test();

  public:
    explicit CLIApplication(int argc, char **argv);
    CLIApplication() = delete;

    auto exec() -> int;
};
} // namespace Later
