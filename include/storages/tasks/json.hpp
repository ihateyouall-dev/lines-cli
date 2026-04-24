#pragma once

#include "lines/tasks/task.hpp"
#include "nlohmann/json_fwd.hpp"

#include <cstddef>
#include <filesystem>

namespace Lines {
namespace TasksJSON {
auto to_json(const Lines::Task &task) -> nlohmann::json;

auto from_json(const nlohmann::json &json) -> Lines::Task;
} // namespace TasksJSON
class TasksJSONStorage {
    std::vector<Task> _tasks;
    std::filesystem::path _file;
    std::size_t last_id{};

  public:
    using iterator = decltype(_tasks)::iterator;
    using const_iterator = decltype(_tasks)::const_iterator;

    TasksJSONStorage(std::filesystem::path file) : _file(std::move(file)) {} // NOLINT
    void load_from_json(const nlohmann::json &json);
    void load_from_file();

    auto tasks() -> std::vector<Task> &;

    [[nodiscard]] auto tasks() const -> const std::vector<Task> &;

    [[nodiscard]] auto to_json() const -> nlohmann::json;

    void save_to_file() const;

    auto operator[](std::size_t index) -> Task &;
    auto operator[](std::size_t index) const -> const Task &;

    auto at(std::size_t index) -> Task &;
    [[nodiscard]] auto at(std::size_t index) const -> const Task &;

    auto add(const Task &task) -> Task &;

    void erase(std::ptrdiff_t index);
    void erase(iterator it);

    auto begin() -> iterator;
    auto end() -> iterator;
    [[nodiscard]] auto begin() const -> const_iterator;
    [[nodiscard]] auto end() const -> const_iterator;

    [[nodiscard]] auto cbegin() const -> const_iterator;
    [[nodiscard]] auto cend() const -> const_iterator;

    auto size() -> std::size_t;

    auto empty() -> bool;
};

} // namespace Lines
