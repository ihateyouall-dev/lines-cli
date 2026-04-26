include(FetchContent)

FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG v2.6.1
)

FetchContent_Declare(
  Lines
  GIT_REPOSITORY https://github.com/ihateyouall-dev/lines-core
  GIT_TAG main
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
add_library(storages_utils)
add_library(cli_tasks)
add_library(client_utils)

target_sources(cli_root PRIVATE cli/root.cpp)
target_sources(cli_tasks PRIVATE cli/tasks/tasks.cpp cli/tasks/filter.cpp)
target_sources(tasks_json_storage PRIVATE storages/tasks/json.cpp)

file(GLOB STORAGES_UTILS_SOURCES "storages/utils/*.cpp")
file(GLOB CLIENT_UTILS_SOURCES "client-utils/*.cpp")

target_sources(storages_utils PRIVATE ${STORAGES_UTILS_SOURCES})
target_sources(client_utils PRIVATE ${CLIENT_UTILS_SOURCES})

target_include_directories(cli_root PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(cli_tasks PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(tasks_json_storage PUBLIC
  ${PROJECT_SOURCE_DIR}/include)
target_include_directories(storages_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(cli_tasks PUBLIC
  CLI11 Lines::Tasks tasks_json_storage client_utils)
target_link_libraries(cli_root PUBLIC CLI11 Lines::Lines cli_tasks)
target_link_libraries(tasks_json_storage PUBLIC
  nlohmann_json::nlohmann_json
  Lines::Tasks
  storages_utils
  client_utils)
target_link_libraries(client_utils PUBLIC Lines::Temporal Lines::Tasks)

add_library(Lines::CLI::Root ALIAS cli_root)
add_library(Lines::Storages::Tasks::Json ALIAS tasks_json_storage)
add_library(Lines::Storages::Utils ALIAS storages_utils)
