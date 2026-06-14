#pragma once

#include "client-utils/id.hpp"
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

  public:
    using iterator = decltype(_tasks)::iterator;
    using const_iterator = decltype(_tasks)::const_iterator;

    using size_type = decltype(_tasks)::size_type;

    using ID = ClientUtils::ID<size_type, 0>;

    explicit TasksJSONStorage(std::filesystem::path file)
        : _file(std::move(file)) {}
    void load_from_json(const nlohmann::json &json);
    void load_from_file();

    [[nodiscard]] auto tasks() const -> const std::vector<Task> &;

    [[nodiscard]] auto to_json() const -> nlohmann::json;

    void save_to_file() const;

    auto operator[](const ID &index) -> Task &;
    auto operator[](const ID &index) const -> const Task &;

    auto at(const ID &index) -> Task &;
    [[nodiscard]] auto at(const ID &index) const -> const Task &;

    auto add(const Task &task) -> Task &;

    void erase(const ID &index);
    void erase(iterator it);

    auto begin() -> iterator;
    auto end() -> iterator;
    [[nodiscard]] auto begin() const -> const_iterator;
    [[nodiscard]] auto end() const -> const_iterator;

    [[nodiscard]] auto cbegin() const -> const_iterator;
    [[nodiscard]] auto cend() const -> const_iterator;

    auto size() -> size_type;

    auto empty() -> bool;
};

} // namespace Lines
