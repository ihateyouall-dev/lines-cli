#include "cli/tasks/tasks.hpp"

#include "CLI/CLI.hpp"
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

using namespace Lines::ClientUtils;

namespace {
void disable_task_repeat_rule(Lines::Task &task) { task.set_repeat_rule(std::nullopt); }
void disable_task_repeat_end(Lines::Task &task) {
    if (!task.repeat_rule() || !task.repeat_rule()->end) {
        throw std::logic_error(
            "ERROR: Cannot disable repeat end from task without repeat rule or repeat end");
    }
    auto rr = *task.repeat_rule();
    rr.end = std::nullopt;
    task.set_repeat_rule(rr);
}
void disable_task_deadline(Lines::Task &task) {
    if (task.repeat_rule()) {
        throw std::logic_error("ERROR: Cannot disable deadline from task with repeat rule");
    }
    task.set_deadline(std::nullopt);
}

void complete_or_advance_deadline(Lines::Task &task) { // NOLINT
    if (task.repeat_rule()) {
        task.advance_deadline();
        // Disabling repeat rule when its end reached
        if (!task.deadline()) {
            task.set_repeat_rule(std::nullopt);
        }
    } else {
        task.complete();
    }
}

template <typename Fn> void with_validation(const Fn &fn) {
    try {
        fn();
    } catch (const std::exception &e) {
        throw CLI::ValidationError(e.what());
    }
}

void validate_regex(std::string_view regex) {
    re2::RE2 r{regex, re2::RE2::Quiet};
    if (!r.ok()) {
        throw std::invalid_argument(std::format("REGEX ERROR: {}", r.error()));
    }
}
} // namespace

Lines::CLI::Tasks::Tasks() { _storage.load_from_file(); }; // NOLINT

auto Lines::CLI::Tasks::require_task(std::size_t index) -> Lines::Task * {
    Lines::Task *result = nullptr;
    try {
        result = &_storage.at(index);
    } catch (const std::out_of_range &) {
        std::cerr << "ERROR: Task not found\n";
    }
    return result;
}

void Lines::CLI::Tasks::showing_init(::CLI::App &app) {
    auto *show = app.add_subcommand("show", "Show information about tasks");

    add_filter_options(*show, "Show");

    show->callback([this]() -> void { showing_callback(); });
}

void Lines::CLI::Tasks::addition_init(::CLI::App &app) {
    auto *add = app.add_subcommand("add", "Add the task");
    add->add_option("title", _options.title, "Give task a title")->required();
    add_task_options(*add, "Give task a");

    add->callback([this]() -> void { addition_callback(); });
}

void Lines::CLI::Tasks::editing_init(::CLI::App &app) {
    auto *edit = app.add_subcommand("edit", "Edit task");
    edit->add_option("-i,--id", _options.tasks_filter_rule.id, "Edit task with given ID")
        ->required();
    edit->add_option("--title", _options.title, "Give task a new title");

    add_task_options(*edit, "Give task a new",
                     TaskOptionsFormats{.timepoint_format = timepoint_format,
                                        .disabling_annot = ". Enter \'none\' to disable it"});
    add_force_flag(*edit, "editing");

    edit->callback([this]() -> void { editing_callback(); });
}

void Lines::CLI::Tasks::deletion_init(::CLI::App &app) {
    auto *delete_app = app.add_subcommand("delete", "Delete tasks");

    add_filter_options(*delete_app, "Delete");

    delete_app->get_option_group("filters")->require_option(1, 0);

    add_force_flag(*delete_app, "deletion");

    delete_app->callback([this]() -> void { deletion_callback(); });
}

void Lines::CLI::Tasks::completion_init(::CLI::App &app) {
    auto *complete = app.add_subcommand("complete", "Complete tasks");
    auto *uncomplete = app.add_subcommand("uncomplete", "Uncomplete tasks");

    add_filter_options(*complete, "Complete");
    add_filter_options(*uncomplete, "Uncomplete");

    add_force_flag(*complete, "completion");
    add_force_flag(*uncomplete, "uncompletion");

    complete->get_option_group("filters")->require_option(1, 0);
    uncomplete->get_option_group("filters")->require_option(1, 0);

    complete->callback([this]() -> void {
        completion_callback([](auto &task) -> void { complete_or_advance_deadline(task); },
                            [](const auto &task) -> bool { return task.completed(); }, "complete");
    });
    uncomplete->callback([this]() -> void {
        completion_callback([](auto &task) -> void { task.uncomplete(); },
                            [](const auto &task) -> bool { return !task.completed(); },
                            "uncomplete");
    });
}

void Lines::CLI::Tasks::init(::CLI::App &app) {
    auto *tasks = app.add_subcommand("tasks", "Work with tasks");
    addition_init(*tasks);
    completion_init(*tasks);
    showing_init(*tasks);
    deletion_init(*tasks);
    editing_init(*tasks);
}

void Lines::CLI::Tasks::save() {
    _storage.save_to_file();
    _dirty = false;
}

auto Lines::CLI::Tasks::dirty() const -> bool { return _dirty; };

void Lines::CLI::Tasks::add_filter_options(::CLI::App &app, std::string_view desc_prefix) {
    auto *filters = app.add_option_group("filters");
    filters->add_option("-i,--id", _options.tasks_filter_rule.id,
                        std::format("{} task with given id", desc_prefix));
    filters
        ->add_option_function<std::string>(
            "--title",
            [this](const std::string &regex) -> void {
                with_validation([&]() -> void {
                    validate_regex(regex);
                    _options.tasks_filter_rule.title_regex.emplace(regex);
                });
            },
            std::format("{} tasks whose titles matches given regular expression", desc_prefix))
        ->type_name("REGEX");
    filters
        ->add_option_function<std::string>(
            "--title-p",
            [this](const std::string &regex) -> void {
                with_validation([&]() -> void {
                    validate_regex(regex);
                    _options.tasks_filter_rule.partial_title_regex.emplace(regex);
                });
            },
            std::format("{} tasks whose titles partially matches given regular expression",
                        desc_prefix))
        ->type_name("REGEX");

    filters->add_flag("-a,--all", _options.tasks_filter_rule.all,
                      std::format("{} all tasks", desc_prefix));
    // Tag specific filters
    filters->add_option(
        "-T,--any-tag", _options.tasks_filter_rule.any_tag,
        std::format("{} only tasks that have at least one of given tags", desc_prefix));
    filters->add_option("-A,--all-tags", _options.tasks_filter_rule.all_tags,
                        std::format("{} only tasks that have all of given tags", desc_prefix));
    // Time point specific filters
    filters
        ->add_option_function<std::string>(
            "-D,--deadline",
            [this](const std::string &date) -> void {
                with_validation([&]() -> void {
                    _options.tasks_filter_rule.deadline = Parsers::parse_timepoint(date);
                });
            },
            std::format("{} task with given deadline (format: YYYY.MM.DD_[HH:MM[:SS]])",
                        desc_prefix))
        ->type_name("TIMEPOINT");

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

void Lines::CLI::Tasks::add_force_flag(::CLI::App &app, std::string_view desc_postfix) {
    app.add_flag("-f,--force", _options.force, std::format("Force {}", desc_postfix));
}

void Lines::CLI::Tasks::addition_callback() {
    if (!_options.title) {
        throw ::CLI::ValidationError("ERROR: Task title cannot be empty");
    }

    Lines::Task task{Lines::TaskInfo{*_options.title, _options.description,
                                     _options.tags.value_or(std::vector<std::string>{})}};

    with_validation([&]() -> void {
        if (_options.deadline) {
            task.set_deadline(Parsers::parse_timepoint(*_options.deadline));
        }

        if (_options.repeat_rule) {
            enable_task_repeat_rule(task);
        }

        if (_options.repeat_end) {
            enable_task_repeat_end(task);
        }
    });

    std::size_t id = _storage.size();
    std::cout << std::format("Added task:\nID: {}\n{}\n", id + 1, task_str_unfolded(task));
    _storage.add(task);
    _dirty = true;
}

void Lines::CLI::Tasks::editing_callback() {
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
        if (*_options.deadline == disable) {
            with_validation([&]() -> void { disable_task_deadline(tmp); });
        } else {
            with_validation(
                [&]() -> void { tmp.set_deadline(Parsers::parse_timepoint(*_options.deadline)); });
        }
    }
    if (_options.repeat_rule) {
        if (_options.repeat_rule == disable) {
            disable_task_repeat_rule(tmp);
        } else {
            with_validation([&]() -> void {
                tmp.uncomplete();
                enable_task_repeat_rule(tmp);
            });
        }
    }
    if (_options.repeat_end) {
        if (*_options.repeat_end == disable) {
            with_validation([&]() -> void { disable_task_repeat_end(tmp); });
        } else {
            with_validation([&]() -> void { enable_task_repeat_end(tmp); });
        }
    }
    std::cout << std::format("Edited task:\n{}\n", task_str_unfolded(tmp));
    if (!_options.force && !confirm()) {
        return;
    }
    *task = tmp;
    _dirty = true;
}

void Lines::CLI::Tasks::showing_callback() {
    if (_options.tasks_filter_rule.id) {
        --*_options.tasks_filter_rule.id;
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "Seems like there's no tasks\n";
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

void Lines::CLI::Tasks::deletion_callback() {
    if (_options.tasks_filter_rule.id) {
        --*_options.tasks_filter_rule.id;
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "ERROR: Task not found\n";
        return;
    }
    if (tasks.size() == 1) {
        std::cout << std::format("Task to delete:\nID: {}\n{}\n", tasks[0].id + 1,
                                 task_str_unfolded(*tasks[0].task));
    } else {
        std::cout << "Tasks to delete:\n";
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

void Lines::CLI::Tasks::add_task_options(::CLI::App &app, std::string_view desc_prefix, // NOLINT
                                         const TaskOptionsFormats &formats) {
    app.add_option("-d,--description", _options.description,
                   std::format("{} description", desc_prefix));
    app.add_option("-t,--tags", _options.tags, std::format("{} tags", desc_prefix));

    app.add_option("-D,--deadline", _options.deadline,
                   std::format("{} planned deadline. Format: {}{}", desc_prefix,
                               formats.timepoint_format, formats.disabling_annot))
        ->type_name("TIMEPOINT");
    app.add_option("-R,--repeat", _options.repeat_rule,
                   std::format("{} repeat rule{}", desc_prefix, formats.disabling_annot))
        ->type_name("REPEAT RULE");
    app.add_option("--rend,--repeat-end", _options.repeat_end,
                   std::format("{} end of repeat{}", desc_prefix, formats.disabling_annot))
        ->type_name("TIMEPOINT");
}

void Lines::CLI::Tasks::enable_task_repeat_rule(Lines::Task &task) {
    Lines::TaskRepeatRule rr;
    rr = Parsers::parse_repeat_rule(*_options.repeat_rule);
    task.set_repeat_rule(rr);
    if (!task.deadline()) {
        task.set_deadline(Lines::Temporal::LocalClock::now());
    }
    task.advance_deadline();
}

void Lines::CLI::Tasks::enable_task_repeat_end(Lines::Task &task) {
    if (!task.repeat_rule()) {
        throw std::logic_error("ERROR: Cannot give repeat end to task without repeat rule");
    }
    Lines::TaskRepeatRule rr = *task.repeat_rule();
    rr.end = Lines::ClientUtils::Parsers::parse_timepoint(*_options.repeat_end);
    task.set_repeat_rule(rr);
}
