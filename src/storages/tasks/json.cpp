#include "lines/tasks/task.hpp"
#include "lines/tasks/task_completion.hpp"
#include "lines/tasks/task_info.hpp"
#include <storages/tasks/json.hpp>

auto Lines::TasksJSON::to_json(const Lines::Task &task) -> nlohmann::json {
    nlohmann::json result;
    result["title"] = task.title();
    result["description"] = task.description().value_or("");

    for (const auto &tag : task.tags()) {
        result["tags"].push_back(tag);
    }

    result["state"] = task.completion().state();
    return result;
}

auto Lines::TasksJSON::from_json(const nlohmann::json &json) -> Lines::Task {
    Lines::TaskInfo info;
    info.title = json.at("title").get<std::string>();
    info.description = json.value("description", "");
    if (json.contains("tags")) {
        for (const auto &tag : json["tags"]) {
            info.tags.emplace_back(tag.get<std::string>());
        }
    }

    Lines::TaskCompletion completion;
    if (json.contains("state")) {
        TaskCompletion::State state = json["state"].get<TaskCompletion::State>();
        if (state == TaskCompletion::State::Completed) {
            completion.complete();
        } else if (state == TaskCompletion::State::Skipped) {
            completion.skip();
        } else {
            completion.reset();
        }
    }

    Lines::Task task(info);
    task.completion() = completion;
    return task;
}
