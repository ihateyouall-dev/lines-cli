#pragma once

#include "lines/temporal/date.hpp"
#include "lines/temporal/timepoint.hpp"
#include <vector>

namespace Lines {
class TasksJSONStorage;
class Task;

struct TasksFilterRule {
    using TaskID = std::size_t;
    using TaskTag = std::string;
    std::optional<TaskID> id;
    std::optional<std::vector<TaskTag>> any_tag;
    std::optional<std::vector<TaskTag>> all_tags;
    std::optional<Lines::Temporal::TimePoint> deadline;
    std::optional<bool> active_bool;
    std::optional<Lines::Temporal::TimePoint> active_deadline;
};
struct TasksFilterResult {
    using TaskID = std::size_t;
    TaskID id;
    Lines::Task *task;
};

auto has_tag(const Task &task, const std::string &tag) -> bool;
auto filter(Lines::TasksJSONStorage &storage, const TasksFilterRule &rule)
    -> std::vector<TasksFilterResult>;
} // namespace Lines
