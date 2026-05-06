#include "cli/tasks/tasks.hpp"

#include "CLI/CLI11.hpp"
#include "client-utils/parsers.hpp"
#include "client-utils/utils.hpp"
#include "lines/tasks/task.hpp"
#include "lines/temporal/clocks.hpp"

#include <cctype>
#include <cstddef>
#include <exception>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>

namespace {
void enable_task_repeat_rule(Lines::Task &task, const std::string &rr) {
    task.set_repeat_rule(parse_repeat_rule(rr));
    task.set_deadline(task.deadline().value_or(Lines::Temporal::LocalClock::now()));
    task.advance_deadline();
}

void disable_task_repeat_rule(Lines::Task &task) { task.set_repeat_rule(std::nullopt); }
void disable_task_deadline(Lines::Task &task) { task.set_deadline(std::nullopt); }

void complete_or_advance_deadline(Lines::Task &task) {
    if (task.repeat_rule()) {
        task.advance_deadline();
    } else {
        task.complete();
    }
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

    show->callback([this]() -> void { showing_callback(); });
}

void Tasks::addition_init(CLI::App &app) {
    auto *add = app.add_subcommand("add", "Add the task");
    add->add_option("title", _options.title, "Give task a title")->required();
    add_task_options(*add, "Give task a");

    add->callback([this]() -> void { addition_callback(); });
}

void Tasks::editing_init(CLI::App &app) {
    auto *edit = app.add_subcommand("edit", "Edit task");
    edit->add_option("-i,--id", _options.tasks_filter_rule.id, "Edit task with given ID")
        ->required();
    edit->add_option("--title", _options.title, "Give task a new title");

    add_task_options(*edit, "Give task a new",
                     TaskOptionsFormats{.timepoint_format = timepoint_format,
                                        .disabling_annot = ". Enter \'0\' to disable it"});

    edit->callback([this]() -> void { editing_callback(); });
}

void Tasks::deletion_init(CLI::App &app) {
    auto *delete_app = app.add_subcommand("delete", "Delete tasks");

    add_filter_options(*delete_app, "Delete");

    delete_app->add_flag("-f,--force", _options.force, "Force deletion");

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
    filters->add_option("-i,--id", _options.tasks_filter_rule.id,
                        std::format("{} task with given id", desc_prefix));
    // Tag specific filters
    filters->add_option(
        "-T,--any-tag", _options.tasks_filter_rule.any_tag,
        std::format("{} only tasks that have at least one of given tags", desc_prefix));
    filters->add_option("-A,--all-tags", _options.tasks_filter_rule.all_tags,
                        std::format("{} only tasks that have all of given tags", desc_prefix));
    // Time point specific filters
    filters->add_option_function<std::string>(
        "-D,--deadline",
        [this](const std::string &date) -> void {
            try {
                _options.tasks_filter_rule.deadline = parse_timepoint(date);
            } catch (std::invalid_argument &e) {
                throw CLI::ValidationError(e.what());
            }
        },
        std::format("{} task with given deadline (format: YYYY.MM.DD_[HH:MM[:SS]])", desc_prefix));

    auto active_callback = [this](bool b) { // NOLINT
        return [this, b]() -> void {
            _options.tasks_filter_rule.active_bool = b;
            _options.tasks_filter_rule.active_deadline = Lines::Temporal::LocalClock::now();
        };
    };
    filters->add_flag_callback("--ac,--active", active_callback(true),
                               std::format("{} only active tasks", desc_prefix));
    filters->add_flag_callback("--ex,--expired", active_callback(false),
                               std::format("{} only expired tasks", desc_prefix));
}

void Tasks::addition_callback() {
    if (!_options.title) {
        throw CLI::ValidationError("Added task title cannot be empty");
    }

    Lines::Task task{Lines::TaskInfo{*_options.title, _options.description,
                                     _options.tags.value_or(std::vector<std::string>{})}};

    try {
        if (_options.deadline) {
            task.set_deadline(parse_timepoint(*_options.deadline));
        }
    } catch (const std::exception &e) {
        throw CLI::ValidationError(e.what());
    }

    try {
        if (_options.repeat_rule) {
            enable_task_repeat_rule(task, *_options.repeat_rule);
        }
    } catch (const std::invalid_argument &e) {
        throw CLI::ValidationError(e.what());
    }

    std::size_t id = _storage.size();
    std::cout << std::format("Added task:\nID: {}\n{}\n", id + 1, task_str_unfolded(task));
    _storage.add(task);
    _dirty = true;
}

void Tasks::editing_callback() {
    auto *task = require_task(*_options.tasks_filter_rule.id - 1);
    if (task == nullptr) {
        return;
    }
    auto tmp = *task;
    if (_options.title) {
        tmp.set_title(*_options.title);
    }
    if (_options.description) {
        tmp.set_description(*_options.description);
    }
    if (_options.tags) {
        tmp.set_tags(*_options.tags);
    }
    if (_options.deadline) {
        if (*_options.deadline == "0") {
            disable_task_deadline(tmp);
        } else {
            try {
                tmp.set_deadline(parse_timepoint(*_options.deadline));
            } catch (const std::exception &e) {
                throw CLI::ValidationError(e.what());
            }
        }
    }
    if (_options.repeat_rule) {
        if (_options.repeat_rule == "0") {
            disable_task_repeat_rule(tmp);
        } else {
            try {
                tmp.uncomplete();
                enable_task_repeat_rule(tmp, *_options.repeat_rule);
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
    if (_options.tasks_filter_rule.id) {
        --*_options.tasks_filter_rule.id;
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "Seems like there's no tasks, take break ;)\n";
        return;
    }
    if (tasks.size() == 1) {
        std::cout << std::format("ID: {}\n{}\n", tasks[0].id + 1,
                                 task_str_unfolded(*tasks[0].task));
        return;
    }
    for (const auto &task : tasks) {
        std::cout << std::format("{}. {}\n", task.id + 1, task_str(*task.task));
    }
}

void Tasks::deletion_callback() {
    if (_options.tasks_filter_rule.id) {
        --*_options.tasks_filter_rule.id;
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "ERROR: Task not found\n";
        return;
    }
    if (tasks.size() == 1) {
        std::cout << std::format("Deleted task:\nID: {}\n{}\n", tasks[0].id + 1,
                                 task_str_unfolded(*tasks[0].task));
    } else {
        std::cout << "Deleted tasks:\n";
        for (const auto &task : tasks) {
            std::cout << std::format("{}. {}\n", task.id + 1, task_str(*task.task));
        }
    }

    if (!_options.force) {
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
        if (task.task->completed()) {
            std::cerr << "ERROR: Task already completed\n";
            return;
        }
        complete_or_advance_deadline(*task.task);
        std::cout << std::format("ID: {}\n{}\n", task.id + 1, task_str_unfolded(*task.task));
    } else {
        for (const auto &task : tasks) {
            complete_or_advance_deadline(*task.task);
            std::cout << std::format("{}. {}\n", task.id + 1, task_str(*task.task));
        }
    }
    _dirty = true;
}

void Tasks::uncomplete_callback() {
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
        if (!task.task->completed()) {
            std::cerr << "ERROR: Task not completed\n";
            return;
        }
        task.task->uncomplete();
        std::cout << std::format("ID: {}\n{}\n", task.id + 1, task_str_unfolded(*task.task));
    } else {
        for (const auto &task : tasks) {
            task.task->uncomplete();
            std::cout << std::format("{}. {}\n", task.id + 1, task_str(*task.task));
        }
    }
    _dirty = true;
}

void Tasks::add_task_options(CLI::App &app, std::string_view desc_prefix, // NOLINT
                             const TaskOptionsFormats &formats) {
    app.add_option("-d,--description", _options.description,
                   std::format("{} description", desc_prefix));
    app.add_option("-t,--tags", _options.tags, std::format("{} tags", desc_prefix));

    app.add_option("-D,--deadline", _options.deadline,
                   std::format("{} planned deadline. Format: {}{}", desc_prefix,
                               formats.timepoint_format, formats.disabling_annot));
    app.add_option("-R,--repeat", _options.repeat_rule,
                   std::format("{} repeat rule{}", desc_prefix, formats.disabling_annot));
    app.add_option("--repeat-end", _options.repeat_end,
                   std::format("{} end of repeat{}", desc_prefix, formats.disabling_annot));
}
