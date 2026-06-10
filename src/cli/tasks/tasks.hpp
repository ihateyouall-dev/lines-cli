#include "client-utils/config.hpp"
#include "client-utils/filesystem.hpp"
#include "filter.hpp"
#include "lines/tasks/task.hpp"
#include "storages/tasks/json.hpp"

#include <functional>
#include <string>

namespace CLI {
class App;
}

namespace Lines::CLI {
class TasksCmd { // NOLINT
    // Options that gained from command line
    struct Options {
        std::optional<std::string> title;
        std::optional<std::string> description;
        std::optional<std::vector<std::string>> tags;

        std::optional<std::string> due;

        bool force = false;

        std::optional<std::string> repeat_rule;
        std::optional<std::string> repeat_end;

        TasksFilter::TasksFilterRule tasks_filter_rule;
    } _options;
    TasksJSONStorage _storage{ClientUtils::get_fs_home() / ".lines.d" /
                              "saves" / "tasks.json"};

    ClientUtils::Config _cfg;
    std::string timepoint_format = "YYYY/MM/DD[_HH:MM[:SS]]";
    const std::string disable = "none"; // NOLINT

    bool _dirty = false;

    struct TaskOptionsFormats {
        std::string timepoint_format;
        // Message like "Enter "none" to disable something" in editing
        std::string disabling_annot;
    };
    void
    add_task_options(::CLI::App &app, std::string_view desc_prefix, // NOLINT
                     const TaskOptionsFormats &formats = TaskOptionsFormats{
                         .timepoint_format = "YYYY/MM/DD[_HH:MM[:SS]]",
                         .disabling_annot = ""});
    void add_filter_options(::CLI::App &app, std::string_view desc_prefix);
    void add_force_flag(::CLI::App &app, std::string_view desc_postfix);

    void listcmd_init(::CLI::App &app);
    void showcmd_init(::CLI::App &app);
    void setcmd_init(::CLI::App &app);
    void addcmd_init(::CLI::App &app);
    void removecmd_init(::CLI::App &app);
    void completioncmd_init(::CLI::App &app);

    void listcmd_callback();
    void showcmd_callback();
    void setcmd_callback();
    void removecmd_callback();
    void addcmd_callback();
    void completioncmd_callback(
        const std::function<void(Lines::Task &)>
            &fn /* action to do with tasks */,
        const std::function<bool(const Lines::Task &)> &restriction /* boolean
           predicate, if returns true - callback stops */
        ,
        std::string_view action_desc);

  public:
    TasksCmd();
    TasksCmd(TasksCmd &&) = delete;

    auto operator=(TasksCmd &&) -> TasksCmd & = delete;

    void init(::CLI::App &app);

    void save();
    void set_config(const ClientUtils::Config &cfg);
    [[nodiscard]] auto dirty() const -> bool;

    ~TasksCmd() = default;
};
} // namespace Lines::CLI
