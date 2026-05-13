#pragma once

#include "lines/temporal/timepoint.hpp"

#include <re2/re2.h>
#include <vector>

namespace Lines {
class TasksJSONStorage;
class Task;

namespace TasksFilter {
struct TasksFilterRule {
    using TaskID = std::size_t;
    using TaskTag = std::string;
    std::optional<bool> all;
    std::optional<TaskID> id;
    std::optional<std::vector<TaskTag>> any_tag;
    std::optional<std::vector<TaskTag>> all_tags;
    std::optional<Lines::Temporal::TimePoint> deadline;
    std::optional<bool> active_bool;
    std::optional<Lines::Temporal::TimePoint> active_deadline;
    std::optional<re2::RE2> title_regex;
    std::optional<re2::RE2> partial_title_regex;
};
struct TasksFilterResult {
    using TaskID = std::size_t;
    TaskID id;
    Lines::Task *task;
};

auto has_tag(const Task &task, const std::string &tag) -> bool;
auto filter(Lines::TasksJSONStorage &storage, const TasksFilterRule &rule)
    -> std::vector<TasksFilterResult>;
template <typename Fn>
void for_each_filter_result(const std::vector<TasksFilterResult> &vec, const Fn &fn) {
    for (const auto &el : vec) {
        fn(el);
    }
}
} // namespace TasksFilter
} // namespace Lines
