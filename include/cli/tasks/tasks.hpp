#include "filter.hpp"
#include <lines/tasks/task.hpp>
#include <storages/tasks/json.hpp>

namespace CLI {
class App;
}

class Tasks { // NOLINT
    Lines::TaskInfo _added_task_info;
    struct TaskChanges {
        std::optional<std::string> title;
        std::optional<std::string> description;
        std::optional<std::vector<std::string>> tags;
    } _changed_task_info;
    Lines::TasksJSONStorage _storage{"lines_tasks.json"};
    bool _dirty = false;
    Lines::TasksFilterRule _tasks_filter_rule;

    auto require_task(std::size_t index) -> Lines::Task *;

    void showing_init(CLI::App &app);
    void editing_init(CLI::App &app);
    void addition_init(CLI::App &app);
    void deletion_init(CLI::App &app);
    void completion_init(CLI::App &app);

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
