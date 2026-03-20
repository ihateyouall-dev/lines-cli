#include <cstddef>
#include <fstream>
#include <lines/tasks/task.hpp>
#include <stdexcept>
#include <storages/tasks/json.hpp>

namespace {
auto date_to_string(const Lines::Temporal::Date &date) -> std::string {
    return std::format("{:04}.{:02}.{:02}", int(date.year()), unsigned(date.month()),
                       unsigned(date.day()));
}

auto parse_int(std::string_view sv) -> int {
    int value{};
    auto res = std::from_chars(sv.data(), sv.data() + sv.size(), value); // NOLINT
    if (res.ec != std::errc{}) {
        throw std::invalid_argument("Invalid integer");
    }
    return value;
}

auto date_from_string(std::string_view str) -> Lines::Temporal::Date {
    auto first = str.find('.');
    auto second = str.find('.', first + 1);

    if (first == std::string_view::npos || second == std::string_view::npos) {
        throw std::invalid_argument("Invalid date format, expected YYYY.MM.DD");
    }

    auto year_sv = str.substr(0, first);
    auto month_sv = str.substr(first + 1, second - first - 1);
    auto day_sv = str.substr(second + 1);

    auto year = Lines::Temporal::Year{parse_int(year_sv)};
    auto month = Lines::Temporal::Month(parse_int(month_sv));
    auto day = Lines::Temporal::Day(parse_int(day_sv));

    if (!month.ok()) {
        throw std::invalid_argument("Month must be in [1;12]");
    }

    if (!day.ok()) {
        throw std::invalid_argument("Day must be in [1;31]");
    }

    return {year, month, day};
}
} // namespace

auto Lines::TasksJSON::to_json(const Lines::Task &task) -> nlohmann::json {
    nlohmann::json result;
    result["title"] = task.title();
    result["description"] = task.description().value_or("");
    if (task.deadline()) {
        result["date"] = date_to_string(*task.deadline());
    }

    for (const auto &tag : task.tags()) {
        result["tags"].push_back(tag);
    }

    result["completed"] = task.completed();
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

    bool completed{};
    if (json.contains("completed")) {
        bool completed = json["completed"].get<bool>();
    }

    Lines::Task task(info);

    if (json.contains("date")) {
        task.set_deadline(date_from_string(json["date"].get<std::string>()));
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
    if (index >= size() || index < 0) {
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
