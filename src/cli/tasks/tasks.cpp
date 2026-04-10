#include "CLI/CLI.hpp"
#include "lines/tasks/task.hpp"
#include "lines/temporal/clocks.hpp"
#include "lines/temporal/date.hpp"
#include "lines/temporal/duration.hpp"
#include "lines/temporal/timestamp.hpp"
#include "lines/temporal/ymd.hpp"
#include <cctype>
#include <cli/tasks/tasks.hpp>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>

namespace {
auto confirm() -> bool {
    std::string answer{};
    std::cout << "\n\nAre you sure? [yN]: ";
    std::getline(std::cin, answer);
    if (answer.empty()) {
        return false;
    }
    return std::tolower(static_cast<unsigned char>(answer[0])) == 'y';
}

auto date_str(const Lines::Temporal::Date &date) -> std::string {
    return std::format("{:04}.{:02}.{:02}", int(date.year()), unsigned(date.month()),
                       unsigned(date.day()));
}

auto tags_str(const Lines::Task &task) -> std::string {
    std::string result;
    for (const auto &tag : task.tags()) {
        result += std::format(" #{}", tag);
    }
    return result;
}

auto completion_sign(const Lines::Task &task) -> std::string {
    return std::format("[{}]", (task.completed() ? "X" : " "));
}

auto task_str_unfolded(const Lines::Task &task) -> std::string {
    std::string result = std::format("Title: {}\n", task.title());
    if (task.description()) {
        if (!task.description().value().empty()) {
            result += std::format("Description: {}\n", task.description().value());
        }
    }
    if (!task.tags().empty()) {
        result += std::format("Tags:{}\n", tags_str(task));
    }
    if (task.deadline()) {
        result += std::format("Date: {}\n", date_str(*task.deadline()));
    }
    result += '\n' + completion_sign(task);
    return result;
}

auto task_str(const Lines::Task &task) -> std::string {
    std::string res = std::format("{} ", completion_sign(task));
    if (task.deadline()) {
        res += std::format("{} ", date_str(*task.deadline()));
    }
    res += task.title();
    if (!task.tags().empty()) {
        res += tags_str(task);
    }
    return res;
}

auto parse_date(const std::string &str) -> Lines::Temporal::Date {
    std::stringstream ss(str);
    int year{};
    uint32_t month{};
    uint32_t day{};
    char divider{};

    ss >> year >> divider >> month >> divider >> day;

    return {Lines::Temporal::Year{year}, Lines::Temporal::Month{month}, Lines::Temporal::Day{day}};
}

auto parse_time(const std::string &str) -> Lines::Temporal::Timestamp {
    std::stringstream ss(str);
    int hours{};
    int minutes{};
    int seconds{};
    char divider{};

    ss >> hours >> divider >> minutes >> divider >> seconds;

    return {Lines::Temporal::Hours{hours}, Lines::Temporal::Minutes{minutes},
            Lines::Temporal::Seconds{seconds}};
}

auto today() -> Lines::Temporal::Date { return Lines::Temporal::LocalClock::today(); }
auto today_str() -> std::string { return date_str(today()); }

auto tomorrow() -> Lines::Temporal::Date { return today() + Lines::Temporal::Days{1}; }
auto tomorrow_str() -> std::string { return date_str(tomorrow()); }
} // namespace

Tasks::Tasks() { _storage.load_from_file(); }; // NOLINT

auto Tasks::require_task(std::size_t index) -> Lines::Task * {
    Lines::Task *result = nullptr;
    try {
        result = &_storage.at(index);
    } catch (const std::out_of_range &) {
        std::cerr << "ERROR: Task not found\n";
    }
    return result;
}

void Tasks::showing_init(CLI::App &app) {
    auto *show = app.add_subcommand("show", "Show information about tasks");

    add_filter_options(*show, "Show");

    show->callback([this]() -> void { showing_callback(); });
}

void Tasks::addition_init(CLI::App &app) {
    auto *add = app.add_subcommand("add", "Add the task");
    add->add_option("title", _added_task_info.title, "Give task a title")->required();
    add->add_option("-d,--description", _added_task_info.description, "Give task a description");
    add->add_option("-t,--tags", _added_task_info.tags, "Give task a tags");

    add->add_option("-D,--date", _added_task_deadline_date,
                    "Give task a planned date (format: YYYY.MM.DD)");
    add->add_flag_callback(
        "--td,--today", [this]() -> void { _added_task_deadline_date = today_str(); },
        "Same as --date * TODAY *");
    add->add_flag_callback(
        "--tm,--tomorrow", [this]() -> void { _added_task_deadline_date = tomorrow_str(); },
        "Same as --date * TOMORROW *");

    add->callback([this]() -> void { addition_callback(); });
}

void Tasks::editing_init(CLI::App &app) {
    auto *edit = app.add_subcommand("edit", "Edit task");
    edit->add_option("-i,--id", _tasks_filter_rule.id, "Edit task with given number")->required();
    edit->add_option("--title", _changed_task_info.title, "Give task a new title");
    edit->add_option("-d,--description", _changed_task_info.description,
                     "Give task a new description");
    edit->add_option("-t,--tags", _changed_task_info.tags, "Give task a new tags");

    // Date & time editing
    edit->add_option(
        "-D,--date", _changed_task_info.deadline_date,
        "Give task a new date (format: YYYY.MM.DD, Enter \"0\" to just disable existing date)");
    edit->add_flag_callback(
        "--td,--today", [this]() -> void { _changed_task_info.deadline_date = today_str(); },
        "Same as --date * TODAY *");

    edit->add_flag_callback(
        "--tm,--tomorrow", [this]() -> void { _changed_task_info.deadline_date = tomorrow_str(); },
        "Same as --date * TOMORROW *");

    edit->callback([this]() -> void { editing_callback(); });
}

void Tasks::deletion_init(CLI::App &app) {
    auto *delete_app = app.add_subcommand("delete", "Delete tasks");

    add_filter_options(*delete_app, "Delete");

    delete_app->add_flag("-f,--force", _force, "Force deletion");

    delete_app->callback([this]() -> void { deletion_callback(); });
}

void Tasks::completion_init(CLI::App &app) {
    auto *complete = app.add_subcommand("complete", "Complete tasks");
    auto *uncomplete = app.add_subcommand("uncomplete", "Uncomplete tasks");

    add_filter_options(*complete, "Complete");
    add_filter_options(*uncomplete, "Uncomplete");

    complete->callback([this]() -> void { complete_callback(); });
    uncomplete->callback([this]() -> void { uncomplete_callback(); });
}

void Tasks::init(CLI::App &app) {
    auto *tasks = app.add_subcommand("tasks", "Work with tasks");
    addition_init(*tasks);
    completion_init(*tasks);
    showing_init(*tasks);
    deletion_init(*tasks);
    editing_init(*tasks);
}

void Tasks::save() {
    _storage.save_to_file();
    _dirty = false;
}

auto Tasks::dirty() const -> bool { return _dirty; };

void Tasks::add_filter_options(CLI::App &app, const std::string_view &desc_prefix) {
    auto *filters = app.add_option_group("filters");
    filters->add_option("-i,--id", _tasks_filter_rule.id,
                        std::format("{} task with given id", desc_prefix));
    // Tag specific filters
    filters->add_option(
        "-T,--any-tag", _tasks_filter_rule.any_tag,
        std::format("{} only tasks that have at least one of given tags", desc_prefix));
    filters->add_option("-A,--all-tags", _tasks_filter_rule.all_tags,
                        std::format("{} only tasks that have all of given tags", desc_prefix));
    // Date & time(TODO) specific filters
    filters->add_option_function<std::string>(
        "-D,--date",
        [this](const std::string &date) -> void {
            try {
                _tasks_filter_rule.date = parse_date(date);
            } catch (std::invalid_argument &e) {
                throw CLI::ValidationError(e.what());
            }
        },
        std::format("{} task with given date (format: YYYY.MM.DD)", desc_prefix));
    filters->add_flag_callback(
        "--td,--today", [this]() -> void { _tasks_filter_rule.date = today(); },
        std::format("{} only tasks that planned on today", desc_prefix));
    filters->add_flag_callback(
        "--tm,--tomorrow", [this]() -> void { _tasks_filter_rule.date = tomorrow(); },
        std::format("{} only tasks that planned on tomorrow", desc_prefix));

    auto active_callback = [this](bool b) { // NOLINT
        return [this, b]() -> void {
            _tasks_filter_rule.active_bool = b;
            _tasks_filter_rule.active_date = today();
        };
    };
    filters->add_flag_callback("--ac,--active", active_callback(true),
                               std::format("{} only active tasks", desc_prefix));
    filters->add_flag_callback("--ex,--expired", active_callback(false),
                               std::format("{} only expired tasks", desc_prefix));
}

void Tasks::addition_callback() {
    Lines::Task task{_added_task_info};
    try {
        if (_added_task_deadline_date) {
            task.set_deadline(parse_date(*_added_task_deadline_date));
        }
    } catch (const std::invalid_argument &e) {
        throw CLI::ValidationError(e.what());
    }
    std::size_t id = _storage.size();
    std::cout << std::format("Added task:\nID: {}\n{}\n", id, task_str_unfolded(task));
    _storage.add(task);
    _dirty = true;
}

void Tasks::editing_callback() {
    auto *task = require_task(*_tasks_filter_rule.id);
    if (task == nullptr) {
        return;
    }
    auto tmp = *task;
    if (_changed_task_info.title) {
        tmp.set_title(*_changed_task_info.title);
    }
    if (_changed_task_info.description) {
        tmp.set_description(*_changed_task_info.description);
    }
    if (_changed_task_info.tags) {
        tmp.set_tags(*_changed_task_info.tags);
    }
    if (_changed_task_info.deadline_date) {
        if (*_changed_task_info.deadline_date == "0") {
            tmp.set_deadline(std::nullopt);
        } else {
            try {
                tmp.set_deadline(parse_date(*_changed_task_info.deadline_date));
            } catch (const std::invalid_argument &e) {
                throw CLI::ValidationError(e.what());
            }
        }
    }
    std::cout << std::format("Edited task:\n{}", task_str_unfolded(tmp));
    bool confirmed = confirm();
    if (!confirmed) {
        return;
    }
    *task = tmp;
    _dirty = true;
}

void Tasks::showing_callback() {
    auto tasks = filter(_storage, _tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "Seems like there's no tasks, take break ;)\n";
        return;
    }
    if (tasks.size() == 1) {
        std::cout << std::format("ID: {}\n{}\n", tasks[0].id, task_str_unfolded(*tasks[0].task));
        return;
    }
    for (const auto &task : tasks) {
        std::cout << std::format("{}. {}\n", task.id, task_str(*task.task));
    }
}

void Tasks::deletion_callback() {
    auto tasks = filter(_storage, _tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "ERROR: Task not found\n";
        return;
    }
    if (tasks.size() == 1) {
        std::cout << std::format("Deleted task:\nID: {}\n{}\n", tasks[0].id,
                                 task_str_unfolded(*tasks[0].task));
    } else {
        std::cout << "Deleted tasks:\n";
        for (const auto &task : tasks) {
            std::cout << std::format("{}. {}\n", task.id, task_str(*task.task));
        }
    }

    if (!_force) {
        bool confirmed = confirm();
        if (!confirmed) {
            return;
        }
    }

    for (const auto &task : std::ranges::reverse_view(tasks)) {
        _storage.erase(static_cast<std::ptrdiff_t>(task.id));
    }
    _dirty = true;
}

void Tasks::complete_callback() {
    auto tasks = filter(_storage, _tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "ERROR: Task not found\n";
        return;
    }
    if (tasks.size() == 1) {
        auto task = tasks[0];
        if (task.task->completed()) {
            std::cerr << "ERROR: Task already completed\n";
            return;
        }
        task.task->complete();
        std::cout << std::format("ID: {}\n{}\n", task.id, task_str_unfolded(*task.task));
    } else {
        for (const auto &task : tasks) {
            task.task->complete();
            std::cout << std::format("{}. {}\n", task.id, task_str(*task.task));
        }
    }
    _dirty = true;
}

void Tasks::uncomplete_callback() {
    auto tasks = filter(_storage, _tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "ERROR: Task not found\n";
        return;
    }
    if (tasks.size() == 1) {
        auto task = tasks[0];
        if (!task.task->completed()) {
            std::cerr << "ERROR: Task not completed\n";
            return;
        }
        task.task->uncomplete();
        std::cout << std::format("ID: {}\n{}\n", task.id, task_str_unfolded(*task.task));
    } else {
        for (const auto &task : tasks) {
            task.task->uncomplete();
            std::cout << std::format("{}. {}\n", task.id, task_str(*task.task));
        }
    }
    _dirty = true;
}
