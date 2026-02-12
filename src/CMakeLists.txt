include(FetchContent)

FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG v2.6.1
)

FetchContent_Declare(
  Lines
  GIT_REPOSITORY https://github.com/ihateyouall-dev/lines-core
  GIT_TAG v1.0
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(CLI11)
FetchContent_MakeAvailable(Lines)
FetchContent_MakeAvailable(nlohmann_json)

add_library(cli_root)
add_library(tasks_json_storage)

target_sources(cli_root PRIVATE cli/root.cpp)
target_sources(tasks_json_storage PRIVATE storages/tasks/json.cpp)

target_include_directories(cli_root PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tasks_json_storage PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(cli_root PUBLIC CLI11 Lines::Lines)
target_link_libraries(tasks_json_storage PUBLIC nlohmann_json::nlohmann_json Lines::Tasks)

add_library(Lines::CLI::Root ALIAS cli_root)
add_library(Lines::Storages::Tasks::Json ALIAS cli_root)
