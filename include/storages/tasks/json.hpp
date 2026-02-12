#pragma once

#include <cstddef>
#include <lines/tasks/task.hpp>
#include <nlohmann/json.hpp>

namespace Lines::TasksJSON {
auto to_json(const Lines::Task &task) -> nlohmann::json;
auto from_json(const nlohmann::json &json) -> Lines::Task;
} // namespace Lines::TasksJSON
