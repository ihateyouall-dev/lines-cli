#include "cli/tasks/tasks.hpp"

#include "CLI/CLI.hpp"
#include "cli/tasks/filter.hpp"
#include "client-utils/parsers.hpp"
#include "client-utils/utils.hpp"
#include "lines/tasks/task.hpp"
#include "lines/temporal/clocks.hpp"

#include <cstddef>
#include <exception>
#include <format>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>

using namespace Lines::ClientUtils;

namespace {
// Executes function and throws CLI::ValidationError if exception was thrown
// inside body
template <typename Fn, typename Exc = std::exception>
void with_validation(const Fn &fn) {
    try {
        fn();
    } catch (const Exc &e) {
        throw CLI::ValidationError(error_str("ERROR:", e.what()));
    }
}

// Throws if RE2 regex is not valid
void validate_regex(std::string_view regex) {
    re2::RE2 r{regex, re2::RE2::Quiet};
    if (!r.ok()) {
        throw std::invalid_argument(r.error());
    }
}

auto backend_id(std::size_t id) -> std::size_t { return --id; }
auto frontend_id(std::size_t id) -> std::size_t { return ++id; }
void make_backend_id(std::size_t &id) { --id; }

auto digits_in_number(std::size_t num) -> std::size_t {
    std::size_t res = 1;
    while (num >= 10) {
        num /= 10;
        ++res;
    }

    return res;
}

void print_list(const std::span<Lines::TasksFilter::TasksFilterResult> &list,
                bool use_unicode, bool colorize) {
    auto max_id =
        std::ranges::max(list, {}, &Lines::TasksFilter::TasksFilterResult::id);
    auto max_id_width = digits_in_number(max_id.id);

    for (const auto &task : list) {
        auto id_width = digits_in_number(frontend_id(task.id));
        auto delta = max_id_width - id_width;

        // Alignment for output
        for (std::size_t i{}; i < delta; ++i) {
            std::cout << ' ';
        }

        std::cout << std::format("{}. {}\n", frontend_id(task.id),
                                 task_str(*task.task, use_unicode, colorize));
    }
}
} // namespace

Lines::CLI::TasksCmd::TasksCmd() { _storage.load_from_file(); }; // NOLINT

void Lines::CLI::TasksCmd::listcmd_init(::CLI::App &app) {
    auto *list = app.add_subcommand("list", "List tasks")->alias("ls");

    add_filter_options(*list, "List");

    list->callback([this]() -> void { listcmd_callback(); });
}

void Lines::CLI::TasksCmd::showcmd_init(::CLI::App &app) {
    auto *show =
        app.add_subcommand("show", "Show info about the task")->alias("s");

    show->add_option("ID", _options.tasks_filter_rule.id,
                     "ID of the task to show")
        ->required();

    show->callback([this]() -> void { showcmd_callback(); });
}

void Lines::CLI::TasksCmd::addcmd_init(::CLI::App &app) {
    auto *add = app.add_subcommand("add", "Add the new task")->alias("a");
    add->add_option("title", _options.title, "Give task a title")->required();
    add_task_options(*add, "Give task a");

    add->callback([this]() -> void { addcmd_callback(); });
}

void Lines::CLI::TasksCmd::setcmd_init(::CLI::App &app) {
    auto *set = app.add_subcommand("set", "Edit tasks");
    set->add_option("--title", _options.title, "Give task a new title");
    set->add_option("ID", _options.tasks_filter_rule.id, "ID of task to edit");

    add_task_options(*set, "Give task a new",
                     TaskOptionsFormats{.timepoint_format = timepoint_format,
                                        .disabling_annot =
                                            ". Enter \'none\' to disable it"});
    add_force_flag(*set, "editing");

    set->callback([this]() -> void { setcmd_callback(); });
}

void Lines::CLI::TasksCmd::removecmd_init(::CLI::App &app) {
    auto *remove = app.add_subcommand("remove", "Remove tasks")->alias("rm");

    add_filter_options(*remove, "Remove");

    remove->get_option_group("filters")->require_option(1, 0);

    add_force_flag(*remove, "removing");

    remove->callback([this]() -> void { removecmd_callback(); });
}

void Lines::CLI::TasksCmd::completioncmd_init(::CLI::App &app) {
    auto *finish = app.add_subcommand("finish", "Finish tasks")->alias("f");
    auto *reopen =
        app.add_subcommand("reopen", "Reopen finished tasks")->alias("r");

    add_filter_options(*finish, "Finish");
    add_filter_options(*reopen, "Reopen");

    add_force_flag(*finish, "finishing");
    add_force_flag(*reopen, "reopening");

    finish->get_option_group("filters")->require_option(1, 0);
    reopen->get_option_group("filters")->require_option(1, 0);

    finish->callback([this]() -> void {
        completioncmd_callback(
            [](auto &task) -> void { task.complete(); },
            [](const auto &task) -> bool { return task.completed(); },
            "finish");
    });
    reopen->callback([this]() -> void {
        completioncmd_callback(
            [](auto &task) -> void { task.uncomplete(); },
            [](const auto &task) -> bool { return !task.completed(); },
            "reopen");
    });
}

void Lines::CLI::TasksCmd::init(::CLI::App &app) {
    auto *tasks = app.add_subcommand("tasks", "Work with tasks");
    addcmd_init(*tasks);
    completioncmd_init(*tasks);
    listcmd_init(*tasks);
    showcmd_init(*tasks);
    removecmd_init(*tasks);
    setcmd_init(*tasks);
}

void Lines::CLI::TasksCmd::save() {
    _storage.save_to_file();
    _dirty = false;
}

auto Lines::CLI::TasksCmd::dirty() const -> bool { return _dirty; };

void Lines::CLI::TasksCmd::add_filter_options(::CLI::App &app,
                                              std::string_view desc_prefix) {
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
            std::format(
                "{} tasks whose titles matches given regular expression",
                desc_prefix))
        ->type_name("REGEX");
    filters
        ->add_option_function<std::string>(
            "--title-p",
            [this](const std::string &regex) -> void {
                with_validation([&]() -> void {
                    validate_regex(regex);
                    _options.tasks_filter_rule.partial_title_regex.emplace(
                        regex);
                });
            },
            std::format("{} tasks whose titles partially matches given regular "
                        "expression",
                        desc_prefix))
        ->type_name("REGEX");

    filters->add_flag("-a,--all", _options.tasks_filter_rule.all,
                      std::format("{} all tasks", desc_prefix));
    // Tag specific filters
    filters->add_option(
        "-T,--any-tag", _options.tasks_filter_rule.any_tag,
        std::format("{} only tasks that have at least one of given tags",
                    desc_prefix));
    filters->add_option(
        "-A,--all-tags", _options.tasks_filter_rule.all_tags,
        std::format("{} only tasks that have all of given tags", desc_prefix));
    // Time point specific filters
    filters
        ->add_option_function<std::string>(
            "-D,--due",
            [this](const std::string &date) -> void {
                with_validation([&]() -> void {
                    _options.tasks_filter_rule.due =
                        Parsers::parse_timepoint(date);
                });
            },
            std::format(
                "{} task with given due (format: YYYY.MM.DD_[HH:MM[:SS]])",
                desc_prefix))
        ->type_name("TIMEPOINT");

    auto active_callback = [this](bool b) { // NOLINT
        return [this, b]() -> void {
            _options.tasks_filter_rule.active_bool = b;
            _options.tasks_filter_rule.active_due =
                Lines::Temporal::LocalClock::now();
        };
    };
    filters->add_flag_callback(
        "--ac,--active", active_callback(true),
        std::format("{} only active tasks", desc_prefix));
    filters->add_flag_callback(
        "--ex,--expired", active_callback(false),
        std::format("{} only expired tasks", desc_prefix));
}

void Lines::CLI::TasksCmd::add_force_flag(::CLI::App &app,
                                          std::string_view desc_postfix) {
    app.add_flag("-f,--force", _options.force,
                 std::format("Force {}", desc_postfix));
}

void Lines::CLI::TasksCmd::addcmd_callback() {
    if (!_options.title) {
        throw ::CLI::ValidationError(
            error_str("ERROR:", "Task title cannot be empty"));
    }

    Lines::Task task{
        Lines::TaskInfo{*_options.title, _options.description,
                        _options.tags.value_or(std::vector<std::string>{})}};

    with_validation([&]() -> void {
        if (_options.due) {
            task.set_due(Parsers::parse_timepoint(*_options.due));
        }

        if (_options.repeat_rule) {
            task.set_repeat_rule(
                Parsers::parse_repeat_rule(*_options.repeat_rule));
        }

        if (_options.repeat_end) {
            task.set_repeat_end(Parsers::parse_timepoint(*_options.repeat_end));
        }
    });

    const std::size_t id = _storage.size();
    std::cout << std::format(
        "Added task:\nID: {}\n{}\n", frontend_id(id),
        full_task_str(task, _cfg.cli_use_unicode, _cfg.cli_colorize));
    _storage.add(task);
    _dirty = true;
}

void Lines::CLI::TasksCmd::setcmd_callback() {
    Lines::Task *task = nullptr;

    try {
        task = &_storage.at(backend_id(*_options.tasks_filter_rule.id));
    } catch (const std::exception &e) {
        std::cerr << error_str("ERROR:", "Task not found\n");
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
    if (_options.due) {
        if (*_options.due == disable) {
            with_validation([&]() -> void { tmp.set_due(std::nullopt); });
        } else {
            with_validation([&]() -> void {
                tmp.set_due(Parsers::parse_timepoint(*_options.due));
            });
        }
    }
    if (_options.repeat_rule) {
        if (_options.repeat_rule == disable) {
            tmp.set_repeat_rule(std::nullopt);
        } else {
            with_validation([&]() -> void {
                tmp.uncomplete();
                tmp.set_repeat_rule(
                    Parsers::parse_repeat_rule(*_options.repeat_rule));
            });
        }
    }
    if (_options.repeat_end) {
        if (*_options.repeat_end == disable) {
            with_validation(
                [&]() -> void { tmp.set_repeat_end(std::nullopt); });
        } else {
            with_validation([&]() -> void {
                tmp.set_repeat_end(
                    Parsers::parse_timepoint(*_options.repeat_end));
            });
        }
    }
    std::cout << std::format(
        "Edited task:\n{}\n",
        full_task_str(tmp, _cfg.cli_use_unicode, _cfg.cli_colorize));
    if (!_options.force && !_cfg.always_force && !confirm()) {
        return;
    }
    *task = tmp;
    _dirty = true;
}

void Lines::CLI::TasksCmd::removecmd_callback() {
    if (_options.tasks_filter_rule.id) {
        make_backend_id(*_options.tasks_filter_rule.id);
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << error_str("ERROR:", "Task not found\n");
        return;
    }
    print_list(tasks, _cfg.cli_use_unicode, _cfg.cli_colorize);

    if (!_options.force && !_cfg.always_force) {
        std::cout << std::format("\n{} tasks will be removed\n", tasks.size());
        if (!confirm()) {
            return;
        }
    }

    for (const auto &task : std::ranges::reverse_view(tasks)) {
        _storage.erase(static_cast<std::ptrdiff_t>(task.id));
    }
    std::cout << std::format("\n{} tasks was removed\n", tasks.size());
    _dirty = true;
}

void Lines::CLI::TasksCmd::listcmd_callback() {
    if (_options.tasks_filter_rule.id) {
        make_backend_id(*_options.tasks_filter_rule.id);
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << "Seems like there's no tasks\n";
        return;
    }

    print_list(tasks, _cfg.cli_colorize, _cfg.cli_use_unicode);
}

void Lines::CLI::TasksCmd::showcmd_callback() {
    auto id = backend_id(*_options.tasks_filter_rule.id);
    try {
        auto task = _storage.at(id);
        std::cout << std::format(
            "ID: {}\n{}\n", frontend_id(id),
            full_task_str(task, _cfg.cli_use_unicode, _cfg.cli_colorize));
    } catch (const std::exception &e) {
        std::cerr << error_str("ERROR:", "Task not found\n");
        return;
    }
}

void Lines::CLI::TasksCmd::add_task_options(
    ::CLI::App &app, std::string_view desc_prefix, // NOLINT
    const TaskOptionsFormats &formats) {
    app.add_option("-d,--description", _options.description,
                   std::format("{} description", desc_prefix));
    app.add_option("-t,--tags", _options.tags,
                   std::format("{} tags", desc_prefix));

    app.add_option("-D,--due", _options.due,
                   std::format("{} planned due. Format: {}{}", desc_prefix,
                               formats.timepoint_format,
                               formats.disabling_annot))
        ->type_name("TIMEPOINT");
    app.add_option("-R,--repeat", _options.repeat_rule,
                   std::format("{} repeat rule{}", desc_prefix,
                               formats.disabling_annot))
        ->type_name("REPEAT RULE");
    app.add_option("--rend,--repeat-end", _options.repeat_end,
                   std::format("{} end of repeat{}", desc_prefix,
                               formats.disabling_annot))
        ->type_name("TIMEPOINT");
}

void Lines::CLI::TasksCmd::completioncmd_callback(
    const std::function<void(Lines::Task &)> &fn /* action to do with tasks */,
    const std::function<bool(const Lines::Task &)> &restriction /* boolean
       predicate, if returns true - callback stops */
    ,
    std::string_view action_desc) {
    if (_options.tasks_filter_rule.id) {
        --*_options.tasks_filter_rule.id;
    }
    auto tasks = filter(_storage, _options.tasks_filter_rule);
    if (tasks.empty()) {
        std::cerr << error_str("ERROR:", "Task not found\n");
        return;
    }
    if (tasks.size() == 1) {
        auto task = tasks[0];
        auto tmp = *task.task;
        if (restriction(tmp)) {
            std::cerr << error_str(
                "ERROR:",
                // Put action into past simple, if action is reopening we cannot
                // say 'Task already reopened', so substituding it with 'open'
                std::format("Task already {}ed\n",
                            (action_desc == "reopen" ? "open" : action_desc)));
            return;
        }
        fn(tmp);
        std::cout << std::format(
            "Task to {}:\nID: {}\n{}\n", action_desc, frontend_id(task.id),
            ClientUtils::full_task_str(tmp, _cfg.cli_use_unicode,
                                       _cfg.cli_colorize));
        if (_options.force || ClientUtils::confirm()) {
            fn(*task.task);
        }
    } else {
        std::cout << std::format("Tasks to {}\n", action_desc);
        print_list(tasks, _cfg.cli_use_unicode, _cfg.cli_colorize);
        if (_options.force || ClientUtils::confirm()) {
            for (const auto &task : tasks) {
                fn(*task.task);
            }
        }
    }
    _dirty = true;
}

void Lines::CLI::TasksCmd::set_config(const ClientUtils::Config &cfg) {
    _cfg = cfg;
}
