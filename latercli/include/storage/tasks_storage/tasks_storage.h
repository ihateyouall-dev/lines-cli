#pragma once

#include <filesystem>
#include <latertasks.hpp>
#include <nlohmann/json.hpp>

namespace Later {
class ITasksStorage {
  public:
    virtual void save(const Tasks &tasks) = 0;
    virtual auto load() -> Tasks = 0;
    ITasksStorage() = default;
    ITasksStorage(const ITasksStorage &) = default;
    ITasksStorage(ITasksStorage &&) = delete;
    auto operator=(const ITasksStorage &) -> ITasksStorage & = default;
    auto operator=(ITasksStorage &&) -> ITasksStorage & = delete;
    virtual ~ITasksStorage() = default;
};

class TasksJSONStorage : public ITasksStorage {
    std::filesystem::path _file;

  public:
    explicit TasksJSONStorage(std::filesystem::path file) : _file(std::move(file)) {}
    TasksJSONStorage(const TasksJSONStorage &) = default;
    TasksJSONStorage(TasksJSONStorage &&) = delete;
    auto operator=(const TasksJSONStorage &) -> TasksJSONStorage & = default;
    auto operator=(TasksJSONStorage &&) -> TasksJSONStorage & = delete;
    void save(const Tasks &tasks) override;
    auto load() -> Tasks override;
    ~TasksJSONStorage() override = default;
};
}; // namespace Later
