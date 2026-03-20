#include "lines/tasks/task.hpp"
#include "lines/temporal/date.hpp"
#include "lines/temporal/ymd.hpp"
#include <CLI/CLI.hpp>
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

auto date_to_string(const Lines::Temporal::Date &date) -> std::string {
    return std::format("{:04}.{:02}.{:02}", int(date.year()), unsigned(date.month()),
                       unsigned(date.day()));
}

auto tags_to_string(const Lines::Task &task) -> std::string {
    std::string result;
    for (const auto &tag : task.tags()) {
        result += std::format(" #{}", tag);
    }
    return result;
}

auto completion_sign(const Lines::Task &task) -> std::string {
    return std::format("[{}]", (task.completed() ? "X" : " "));
}

auto task_to_string_unfolded(const Lines::Task &task) -> std::string {
    std::string result = std::format("Title: {}\n", task.title());
    if (task.description()) {
        if (!task.description().value().empty()) {
            result += std::format("Description: {}\n", task.description().value());
        }
    }
    if (!task.tags().empty()) {
        result += std::format("Tags:{}\n", tags_to_string(task));
    }
    if (task.deadline()) {
        result += std::format("Date: {}\n", date_to_string(*task.deadline()));
    }
    result += '\n' + completion_sign(task);
    return result;
}

auto task_to_string(const Lines::Task &task) -> std::string {
    std::string res = std::format("{} ", completion_sign(task));
    if (task.deadline()) {
        res += std::format("{} ", date_to_string(*task.deadline()));
    }
    res += task.title();
    if (!task.tags().empty()) {
        res += tags_to_string(task);
    }
    return res;
}

auto parse_int(std::string_view sv) -> int {
    int value{};
    auto res = std::from_chars(sv.data(), sv.data() + sv.size(), value); // NOLINT
    if (res.ec != std::errc{}) {
        throw std::invalid_argument("Invalid integer");
    }
    return value;
}

auto date_from_string(std::string_view str) -> Lines::Temporal::Date {
    auto first = str.find('.');
    auto second = str.find('.', first + 1);

    if (first == std::string_view::npos || second == std::string_view::npos) {
        throw std::invalid_argument("Invalid date format, expected YYYY.MM.DD");
    }

    auto year_sv = str.substr(0, first);
    auto month_sv = str.substr(first + 1, second - first - 1);
    auto day_sv = str.substr(second + 1);

    auto year = Lines::Temporal::Year{parse_int(year_sv)};
    auto month = Lines::Temporal::Month(parse_int(month_sv));
    auto day = Lines::Temporal::Day(parse_int(day_sv));

    if (!month.ok()) {
        throw std::invalid_argument("Month must be in [1;12]");
    }

    if (!day.ok()) {
        throw std::invalid_argument("Day must be in [1;31]");
    }

    return {year, month, day};
}
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

    show->callback([this]() -> void {
        auto tasks = filter(_storage, _tasks_filter_rule);
        if (tasks.empty()) {
            std::cerr << "ERROR: Task not found\n";
            return;
        }
        if (tasks.size() == 1) {
            std::cout << std::format("ID: {}\n{}\n", tasks[0].id,
                                     task_to_string_unfolded(*tasks[0].task));
            return;
        }
        for (const auto &task : tasks) {
            std::cout << std::format("{}. {}\n", task.id, task_to_string(*task.task));
        }
    });
}

void Tasks::addition_init(CLI::App &app) {
    auto *add = app.add_subcommand("add", "Add the task");
    add->add_option("title", _added_task_info.title, "Give task a title")->required();
    add->add_option("-d,--description", _added_task_info.description, "Give task a description");
    add->add_option("-t,--tags", _added_task_info.tags, "Give task a tags");
    add->add_option("--dd,--date", _added_task_deadline, "Give task a planned date");

    add->callback([this]() -> void {
        Lines::Task task{_added_task_info};
        try {
            if (_added_task_deadline) {
                task.set_deadline(date_from_string(*_added_task_deadline));
            }
        } catch (const std::invalid_argument &e) {
            std::cerr << std::format("ERROR: {}\n", e.what());
            return;
        }
        std::cout << std::format("Added task:\n{}\n", task_to_string_unfolded(task));
        _storage.add(task);
        _dirty = true;
    });
}

void Tasks::editing_init(CLI::App &app) {
    auto *edit = app.add_subcommand("edit", "Edit task");
    edit->add_option("-i,--id", _tasks_filter_rule.id, "Edit task with given number")->required();
    edit->add_option("--title", _changed_task_info.title, "Give task a new title");
    edit->add_option("-d,--description", _changed_task_info.description,
                     "Give task a new description");
    edit->add_option("-t,--tags", _changed_task_info.tags, "Give task a new tags");
    edit->add_option("--dd,--date", _changed_task_info.date,
                     "Give task a new date (Enter \"0\" to just disable existing date)");

    edit->callback([this]() -> void {
        auto *task = require_task(*_tasks_filter_rule.id);
        if (!task) {
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
        if (_changed_task_info.date) {
            if (*_changed_task_info.date == "0") {
                tmp.set_deadline(std::nullopt);
            } else {
                try {
                    tmp.set_deadline(date_from_string(*_changed_task_info.date));
                } catch (const std::invalid_argument &e) {
                    std::cerr << std::format("ERROR: {}\n", e.what());
                    return;
                }
            }
        }
        std::cout << std::format("Edited task:\n{}", task_to_string_unfolded(tmp));
        bool confirmed = confirm();
        if (!confirmed) {
            return;
        }
        *task = tmp;
        _dirty = true;
    });
}

void Tasks::deletion_init(CLI::App &app) {
    auto *delete_app = app.add_subcommand("delete", "Delete tasks");

    add_filter_options(*delete_app, "Delete");

    delete_app->add_flag("-f,--force", _force, "Force deletion");

    delete_app->callback([this]() -> void {
        auto tasks = filter(_storage, _tasks_filter_rule);
        if (tasks.empty()) {
            std::cerr << "ERROR: Task not found\n";
            return;
        }
        if (tasks.size() == 1) {
            std::cout << std::format("Deleted task:\nID: {}\n{}\n", tasks[0].id,
                                     task_to_string_unfolded(*tasks[0].task));
        } else {
            std::cout << "Deleted tasks:\n";
            for (const auto &task : tasks) {
                std::cout << std::format("{}. {}\n", task.id, task_to_string(*task.task));
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
    });
}

void Tasks::completion_init(CLI::App &app) {
    auto *complete = app.add_subcommand("complete", "Complete the task");
    auto *uncomplete = app.add_subcommand("uncomplete", "Uncomplete the task");

    add_filter_options(*complete, "Complete");
    add_filter_options(*uncomplete, "Uncomplete");

    complete->callback([this]() -> void {
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
            std::cout << std::format("ID: {}\n{}\n", task.id, task_to_string_unfolded(*task.task));
        } else {
            for (const auto &task : tasks) {
                task.task->complete();
                std::cout << std::format("{}. {}\n", task.id, task_to_string(*task.task));
            }
        }
        _dirty = true;
    });
    uncomplete->callback([this]() -> void {
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
            std::cout << std::format("ID: {}\n{}\n", task.id, task_to_string_unfolded(*task.task));
        } else {
            for (const auto &task : tasks) {
                task.task->uncomplete();
                std::cout << std::format("{}. {}\n", task.id, task_to_string(*task.task));
            }
        }
        _dirty = true;
    });
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
    filters->add_option(
        "-T,--any-tag", _tasks_filter_rule.any_tag,
        std::format("{} only tasks that have at least one of given tags", desc_prefix));
    filters->add_option("-A,--all-tags", _tasks_filter_rule.all_tags,
                        std::format("{} only tasks that have all of given tags", desc_prefix));
}
