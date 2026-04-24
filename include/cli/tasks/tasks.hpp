#include "filter.hpp"
#include "lines/tasks/task.hpp"
#include "storages/tasks/json.hpp"
#include "storages/utils/filesystem.hpp"

#include <string>

namespace CLI {
class App;
}

class Tasks { // NOLINT
    // Options that gained from command line
    struct Options {
        std::optional<std::string> title;
        std::optional<std::string> description;
        std::optional<std::vector<std::string>> tags;

        std::optional<std::string> deadline;

        bool force = false;

        Lines::TasksFilterRule tasks_filter_rule;
    } _options;
    Lines::TasksJSONStorage _storage{Lines::detail::get_fs_home() / ".lines.d" / "saves" /
                                     "tasks.json"};
    bool _dirty = false;

    auto require_task(std::size_t index) -> Lines::Task *;

    void add_task_options(CLI::App &app, std::string_view desc_prefix, // NOLINT
                          std::string_view format = "");
    void add_filter_options(CLI::App &app, const std::string_view &desc_prefix);

    void showing_init(CLI::App &app);
    void editing_init(CLI::App &app);
    void addition_init(CLI::App &app);
    void deletion_init(CLI::App &app);
    void completion_init(CLI::App &app);

    void showing_callback();
    void editing_callback();
    void addition_callback();
    void deletion_callback();
    void complete_callback();
    void uncomplete_callback();

  public:
    Tasks();
    Tasks(const Tasks &) = default;
    Tasks(Tasks &&) = delete;

    auto operator=(const Tasks &) -> Tasks & = default;
    auto operator=(Tasks &&) -> Tasks & = delete;

    void init(CLI::App &app);

    void save();
    [[nodiscard]] auto dirty() const -> bool;

    ~Tasks() = default;
};
