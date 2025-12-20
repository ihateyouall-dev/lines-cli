#include "latertime.h"
#include <cassert>
#include <latercli.hpp>
#include <tasks_storage.h>

auto Later::CLIApplication::get_executable_path() -> std::filesystem::path {
#if defined(LATER_WINDOWSNT)
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path);

#elif defined(LATER_DARWIN)
    char path[1024]; // NOLINT
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size); // NOLINT
    return std::filesystem::canonical(path);

#elif defined(LATER_UNIX)
    return std::filesystem::canonical("/proc/self/exe");

#endif
}

void Later::CLIApplication::test() { std::cout << Later::Time::now().hh_mm_ss() << '\n'; }

void Later::CLIApplication::print_version() {
    std::cout << "Later version " << LATER_VERSION << '\n';
    std::cout << "Path: " << get_executable_path() << '\n';
}

void Later::CLIApplication::print_tasklist(const TaskList &tasklist, const Filter &filter) {
    for (const auto &task : tasklist) {
        const size_t number = &task - &(*tasklist.begin()) + 1;
        if (!filter.check(task)) {
            continue;
        }
        print_task(task, number);
    }
}

auto Later::CLIApplication::exec() -> int {
    CLI11_PARSE(main_app, _argc, _argv);
    TasksJSONStorage storage(tasks_config_path);

    storage.save(tasks);
    return 0;
}

Later::CLIApplication::CLIApplication(int argc, char **argv) : _argc(argc), _argv(argv) { // NOLINT
    main_app.add_flag_callback("-v,--version", print_version, "Show version and installation path");
    main_app.add_flag_callback("--test", test, "Test function");
    setup_tasks();
}

void Later::CLIApplication::setup_tasks() {
    TasksJSONStorage storage(tasks_config_path);
    try {
        tasks = storage.load();
    } catch (...) {
        tasks = Tasks();
    }
    tasks_app = main_app.add_subcommand("tasks", "Work with tasks");
    setup_tasks_add();
    setup_tasks_list();
    setup_tasks_delete();
    setup_tasks_set();
}

void Later::CLIApplication::setup_tasks_add() {
    assert(tasks_app != nullptr);
    tasks_add = tasks_app->add_subcommand("add", "Add new task");
    tasks_add->add_option("-t,--title", task_args.title, "Set task title");
    tasks_add->add_option("-d,--description", task_args.description, "Set task description");
    tasks_add->add_option("-c,--category", task_args.category, "Set task category");
    tasks_add->callback([&, this]() -> void {
        Task new_task;
        try {
            new_task = Task(task_args.title.value(), task_args.description.value_or(""),
                            task_args.category.value_or(""));
        } catch (const std::exception &e) {
            std::cout << e.what() << '\n';
            return;
        }
        tasks.add_task(new_task);
        std::cout << "Added new task:\n";
        print_task(new_task, tasks.daily_tasks().size());
    });
}

void Later::CLIApplication::setup_tasks_list() {
    assert(tasks_app != nullptr);
    tasks_list = tasks_app->add_subcommand("list", "Show list of daily tasks");
    tasks_list->add_option("-c,--category", filter.category, "Show ONLY tasks with same category");
    auto *completion_filter = tasks_list->add_option_group(
        "Completion filter", "Show ONLY completed or uncompleted tasks");
    completion_filter->add_flag("--completed", filter.completed, "Show ONLY completed tasks");
    completion_filter->add_flag("--uncompleted", filter.uncompleted, "Show ONLY uncompleted tasks");
    completion_filter->require_option(0, 1);

    tasks_list->callback([&, this]() -> void {
        filter.validate();
        std::cout << (filter.completed ? "[COMPLETED] " : "")
                  << (filter.uncompleted ? "[UNCOMPLETED] " : "")
                  << (!filter.category.empty() ? "[" + filter.category + "] " : "")
                  << "Tasks for today:\n";
        print_tasklist(tasks.daily_tasks(), filter);
    });
}

void Later::CLIApplication::print_task(const Task &task, size_t number) {
    std::cout << "[" << (task.completed() ? "X" : " ") << "] " << number << " | " << task.title()
              << " | " << task.description().value_or("None") << " | "
              << task.category().value_or("None") << '\n';
}

void Later::CLIApplication::attach_task_args(CLI::App *app) {
    app->add_option("-t,--title", task_args.title, "Set task title");
    app->add_option("-d,--description", task_args.description, "Set task description");
    app->add_option("-c,--category", task_args.category, "Set task category");
}

void Later::CLIApplication::setup_tasks_delete() {
    assert(tasks_app != nullptr);
    tasks_delete = tasks_app->add_subcommand("delete", "Delete daily task");
    tasks_delete->add_option("-n,--number", task_number, "Delete task by number in list");

    tasks_delete->callback([&]() -> void {
        Task deleted_task;
        try {
            deleted_task = tasks.daily_tasks().at(task_number - 1);
            tasks.delete_task(task_number - 1);
        } catch (std::out_of_range &) {
            std::cerr << "[ERROR] Deletion of task failed: task with same number not found\n";
            return;
        }
        std::cout << "Deleted task:\n";
        print_task(deleted_task, task_number);
    });
}

void Later::CLIApplication::setup_tasks_set() {
    assert(tasks_app != nullptr);
    tasks_set = tasks_app->add_subcommand("set", "Edit daily task");
    attach_task_args(tasks_set);
    tasks_set->add_option("-n,--number", task_number, "Set task number");
    auto *complete_uncomplete = tasks_set->add_option_group("Complete or uncomplete task");
    complete_uncomplete->add_flag("--complete", task_complete, "Complete task");
    complete_uncomplete->add_flag("--uncomplete", task_uncomplete, "Uncomplete task");
    complete_uncomplete->require_option(0, 1);

    tasks_set->callback([&]() -> void {
        assert(!(task_complete && task_uncomplete));
        Task *task = nullptr;
        try {
            task = tasks.get_daily_task(task_number - 1);
        } catch (const std::out_of_range &) {
            std::cerr << "[ERROR] Task not found\n";
            return;
        }
        if (task_args.title.has_value()) {
            task->set_title(task_args.title.value());
        }
        if (task_args.description.has_value()) {
            task->set_description(task_args.description.value());
        }
        if (task_args.category.has_value()) {
            task->set_category(task_args.category.value());
        }
        if (task_complete) {
            if (task->completed()) {
                std::cerr << "[ERROR] Task completed already, use --uncomplete to uncomplete it\n";
                return;
            }
            task->complete();
        } else if (task_uncomplete) {
            if (!task->completed()) {
                std::cerr << "[ERROR] Task not completed, use --complete to complete it\n";
                return;
            }
            task->reset();
        }
        std::cout << "Edited task:\n";
        print_task(*task, task_number);
    });
}
