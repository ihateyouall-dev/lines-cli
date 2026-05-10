#include "cli/tasks/filter.hpp"

#include "storages/tasks/json.hpp"

#include <algorithm>

auto Lines::TasksFilter::has_tag(const Task &task, const std::string &tag) -> bool {
    return std::ranges::find(task.tags(), tag) != task.tags().end();
}

auto Lines::TasksFilter::filter(Lines::TasksJSONStorage &storage, const TasksFilterRule &rule)
    -> std::vector<TasksFilterResult> {
    std::vector<TasksFilterResult> result;
    if (rule.id) {
        auto id = *rule.id;
        if (id < storage.size()) {
            result.emplace_back(id, &storage[id]);
        }
        return result;
    }

    auto pred = [&rule](const Task &task) -> bool {
        bool contains_all_tags =
            !rule.all_tags || std::ranges::all_of(*rule.all_tags, [&](const auto &tag) -> auto {
                return has_tag(task, tag);
            });
        bool contains_any_tag =
            !rule.any_tag || std::ranges::any_of(*rule.any_tag, [&](const auto &tag) -> auto {
                return has_tag(task, tag);
            });
        bool satisfying_date = !rule.deadline || *task.deadline() == *rule.deadline;
        bool satisfying_active =
            !rule.active_bool || task.is_active(*rule.active_deadline) == *rule.active_bool;
        return (contains_all_tags && contains_any_tag && satisfying_date && satisfying_active) ||
               rule.all;
    };

    for (std::size_t i{}; i < storage.size(); ++i) {
        auto *task = &storage[i];
        if (pred(*task)) {
            result.emplace_back(i, task);
        }
    }
    return result;
}
