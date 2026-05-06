#include "storages/tasks/json.hpp"

#include "client-utils/parsers.hpp"
#include "client-utils/utils.hpp"
#include "lines/tasks/task.hpp"
#include "lines/tasks/task_repeat.hpp"
#include "lines/temporal/duration.hpp"
#include "lines/temporal/ymd.hpp"
#include "nlohmann/json.hpp"

#include <cstddef>
#include <fstream>
#include <stdexcept>

auto Lines::TasksJSON::to_json(const Lines::Task &task) -> nlohmann::json {
    nlohmann::json result;
    result["title"] = task.title();
    if (task.description()) {
        result["description"] = task.description();
    }
    if (task.deadline()) {
        result["deadline"] = Lines::ClientUtils::timepoint_str_s(*task.deadline());
    }

    for (const auto &tag : task.tags()) {
        result["tags"].push_back(tag);
    }

    if (task.repeat_rule()) {
        auto rr = *task.repeat_rule();
        try {
            auto rtype = std::get<Lines::TaskRepeat::EveryUnit>(rr.repeat_type);
            result["repeat"]["type"] = 0;
            result["repeat"]["interval"] = rtype.interval.count();
            result["repeat"]["unit"] = rtype.unit_str;
        } catch (std::bad_variant_access &) {
            auto rtype = std::get<Lines::TaskRepeat::EveryWeekday>(rr.repeat_type);
            result["repeat"]["type"] = 1;
            result["repeat"]["weekdays"] = rtype.weekdays;
        }
    } else {
        result["completed"] = task.completed();
    }
    return result;
}

auto Lines::TasksJSON::from_json(const nlohmann::json &json) -> Lines::Task {
    Lines::TaskInfo info;
    info.title = json.at("title").get<std::string>();
    if (json.contains("description")) {
        info.description = json["description"].get<std::string>();
    }
    if (json.contains("tags")) {
        for (const auto &tag : json["tags"]) {
            info.tags.emplace_back(tag.get<std::string>());
        }
    }

    bool completed{};
    if (json.contains("completed")) {
        completed = json["completed"].get<bool>();
    }

    Lines::Task task(info);

    if (json.contains("deadline")) {
        task.set_deadline(
            Lines::ClientUtils::Parsers::parse_timepoint_nv(json["deadline"].get<std::string>()));
    }

    if (json.contains("repeat")) {
        Lines::TaskRepeatRule rr;
        int type = json["repeat"]["type"].get<int>();
        if (type == 0) {
            if (!json["repeat"].contains("interval")) {
                throw std::runtime_error("Lines::TasksJSON::from_json: given json contains repeat "
                                         "rule which must contain interval, but hasn't it");
            }
            Lines::Temporal::Seconds interval{json["repeat"]["interval"].get<uint32_t>()};
            auto unit_str = json["repeat"].value("unit", "s");
            rr.repeat_type =
                Lines::TaskRepeat::EveryUnit{.interval = interval, .unit_str = unit_str};
        } else if (type == 1) {
            if (!json["repeat"].contains("weekdays")) {
                throw std::runtime_error("Lines::TasksJSON::from_json: given json contains repeat "
                                         "rule which must contain weekdays, but hasn't it");
            }
            auto weekdays = json["repeat"]["weekdays"].get<std::vector<Lines::Temporal::Weekday>>();
            rr.repeat_type = Lines::TaskRepeat::EveryWeekday{weekdays};
        } else {
            throw std::runtime_error("Lines::TasksJSON::from_json: unknown repeat type");
        }
        task.set_repeat_rule(rr);
    }

    if (completed) {
        task.complete();
    }
    return task;
}

void Lines::TasksJSONStorage::load_from_json(const nlohmann::json &json) {
    _tasks.clear();
    _tasks.reserve(json.size());

    for (const auto &task : json) {
        _tasks.emplace_back(TasksJSON::from_json(task));
    }
}

void Lines::TasksJSONStorage::load_from_file() {
    std::ifstream fs(_file);

    if (!fs) {
        return;
    }

    nlohmann::json json;
    fs >> json;
    load_from_json(json);
}

auto Lines::TasksJSONStorage::tasks() -> std::vector<Task> & { return _tasks; }

auto Lines::TasksJSONStorage::tasks() const -> const std::vector<Task> & { return _tasks; }

auto Lines::TasksJSONStorage::to_json() const -> nlohmann::json {
    nlohmann::json res;
    for (const auto &task : _tasks) {
        res.push_back(TasksJSON::to_json(task));
    }
    return res;
}

void Lines::TasksJSONStorage::save_to_file() const {
    std::filesystem::create_directories(_file.parent_path());
    std::ofstream fs(_file);

    if (!fs) {
        throw std::runtime_error("Lines::TasksJSONStorage::load_from_file: cannot open file: " +
                                 _file.string());
    }

    fs << to_json().dump(4);
}

auto Lines::TasksJSONStorage::operator[](std::size_t index) -> Lines::Task & {
    return _tasks[index];
}

auto Lines::TasksJSONStorage::operator[](std::size_t index) const -> const Lines::Task & {
    return _tasks[index];
}

auto Lines::TasksJSONStorage::at(std::size_t index) -> Lines::Task & { return _tasks.at(index); }

auto Lines::TasksJSONStorage::at(std::size_t index) const -> const Lines::Task & {
    return _tasks.at(index);
}

void Lines::TasksJSONStorage::erase(std::ptrdiff_t index) {
    if (static_cast<std::size_t>(index) >= size() || index < 0) {
        throw std::out_of_range("Lines::TasksJSONStorage::erase: index out of range");
    }
    _tasks.erase(_tasks.begin() + index);
}

void Lines::TasksJSONStorage::erase(iterator it) { _tasks.erase(it); }

auto Lines::TasksJSONStorage::begin() -> iterator { return _tasks.begin(); }
auto Lines::TasksJSONStorage::end() -> iterator { return _tasks.end(); }

auto Lines::TasksJSONStorage::begin() const -> const_iterator { return _tasks.begin(); }
auto Lines::TasksJSONStorage::end() const -> const_iterator { return _tasks.end(); }

auto Lines::TasksJSONStorage::size() -> std::size_t { return _tasks.size(); }
auto Lines::TasksJSONStorage::empty() -> bool { return _tasks.empty(); }

auto Lines::TasksJSONStorage::cbegin() const -> const_iterator { return _tasks.cbegin(); }
auto Lines::TasksJSONStorage::cend() const -> const_iterator { return _tasks.cend(); }
auto Lines::TasksJSONStorage::add(const Lines::Task &task) -> Task & {
    return _tasks.emplace_back(task);
}
