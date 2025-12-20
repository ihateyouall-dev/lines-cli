add_library(tasks_storage STATIC ../../../src/storage/tasks_storage/tasks_storage.cpp)
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(tasks_storage PUBLIC nlohmann_json::nlohmann_json Later::Tasks)

target_include_directories(tasks_storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tasks_storage)

add_library(Later::Tasks::Storage ALIAS tasks_storage)
