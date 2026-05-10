#include "client-utils/utils.hpp"
#include "filter.hpp"
#include "lines/tasks/task.hpp"
#include "storages/tasks/json.hpp"
#include "storages/utils/filesystem.hpp"

#include <iostream>
#include <string>

namespace CLI {
class App;
}

namespace Lines::CLI {
class Tasks { // NOLINT
    // Options that gained from command line
    struct Options {
        std::optional<std::string> title;
        std::optional<std::string> description;
        std::optional<std::vector<std::string>> tags;

        std::optional<std::string> deadline;

        bool force = false;

        // Options for repeat rules, repeat_end and repeat_times cannot be given both at one time
        std::optional<std::string> repeat_rule;
        std::optional<std::string> repeat_end;
        std::optional<std::size_t> repeat_times;

        TasksFilter::TasksFilterRule tasks_filter_rule;
    } _options;
    Lines::TasksJSONStorage _storage{Lines::detail::get_fs_home() / ".lines.d" / "saves" /
                                     "tasks.json"};
    std::string timepoint_format = "YYYY/MM/DD[_HH:MM[:SS]]";

    bool _dirty = false;

    auto require_task(std::size_t index) -> Lines::Task *;
    void enable_task_repeat_rule(Lines::Task &task);

    struct TaskOptionsFormats {
        std::string timepoint_format;
        // Message like "Enter 0 to disable something" in editing
        std::string disabling_annot;
    };
    void add_task_options(::CLI::App &app, std::string_view desc_prefix, // NOLINT
                          const TaskOptionsFormats &formats = TaskOptionsFormats{
                              .timepoint_format = "YYYY/MM/DD[_HH:MM[:SS]]",
                              .disabling_annot = ""});
    void add_filter_options(::CLI::App &app, std::string_view desc_prefix);
    void add_force_flag(::CLI::App &app, std::string_view desc_postfix);

    void showing_init(::CLI::App &app);
    void editing_init(::CLI::App &app);
    void addition_init(::CLI::App &app);
    void deletion_init(::CLI::App &app);
    void completion_init(::CLI::App &app);

    void showing_callback();
    void editing_callback();
    void addition_callback();
    void deletion_callback();
    template <typename Fn, typename Pred>
    void completion_callback(const Fn &fn, const Pred &restriction, std::string_view action_desc) {
        if (_options.tasks_filter_rule.id) {
            --*_options.tasks_filter_rule.id;
        }
        auto tasks = filter(_storage, _options.tasks_filter_rule);
        if (tasks.empty()) {
            std::cerr << "ERROR: Task not found\n";
            return;
        }
        if (tasks.size() == 1) {
            auto task = tasks[0];
            auto tmp = *task.task;
            if (restriction(tmp)) {
                std::cerr << std::format("ERROR: Task already {}d\n", action_desc);
                return;
            }
            fn(tmp);
            std::cout << std::format("Task to {}:\nID: {}\n{}\n", action_desc, task.id + 1,
                                     ClientUtils::task_str_unfolded(tmp));
            if (ClientUtils::confirm()) {
                fn(*task.task);
            }
        } else {
            std::cout << std::format("Tasks to {}\n", action_desc);
            for (const auto &task : tasks) {
                std::cout << std::format("{}. {}\n", task.id + 1,
                                         ClientUtils::task_str(*task.task));
            }
            if (ClientUtils::confirm()) {
                for (const auto &task : tasks) {
                    fn(*task.task);
                }
            }
        }
        _dirty = true;
    }

  public:
    Tasks();
    Tasks(const Tasks &) = default;
    Tasks(Tasks &&) = delete;

    auto operator=(const Tasks &) -> Tasks & = default;
    auto operator=(Tasks &&) -> Tasks & = delete;

    void init(::CLI::App &app);

    void save();
    [[nodiscard]] auto dirty() const -> bool;

    ~Tasks() = default;
};
} // namespace Lines::CLI
