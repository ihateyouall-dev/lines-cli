#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <tasks_storage.h>

void Later::TasksJSONStorage::save(const Tasks &tasks) {
    nlohmann::ordered_json json = nlohmann::ordered_json::array();
    for (const Task &task : tasks.daily_tasks()) {
        nlohmann::ordered_json task_json = nlohmann::ordered_json::object();
        task_json["title"] = task.title();
        task_json["description"] = task.description().value_or("");
        task_json["category"] = task.category().value_or("");
        task_json["completed"] = task.completed();
        json.emplace_back(task_json);
    }
    std::ofstream file(_file);
    if (!file.is_open()) {
        throw std::invalid_argument("TasksJSONStorage::save: file with same path not existing");
    }
    file << json.dump(4);
}

auto Later::TasksJSONStorage::load() -> Tasks {
    if (!std::filesystem::exists(_file)) {
        throw std::invalid_argument("TasksJSONStorage::load: file not exists");
    }

    std::ifstream file(_file);

    if (!file.is_open()) {
        throw std::runtime_error("TasksJSONStorage::load: cannot open file");
    }
    Tasks tasks;
    nlohmann::ordered_json json = nlohmann::ordered_json::object();
    file >> json;

    for (const auto &task_json : json) {
        Task task;
        task.set_title(task_json["title"]);
        task.set_description(task_json["description"]);
        task.set_category(task_json["category"]);
        if (task_json["completed"]) {
            task.complete();
        }
        tasks.add_task(task);
    }

    return tasks;
}
