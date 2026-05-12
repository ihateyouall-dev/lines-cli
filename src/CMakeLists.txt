include(FetchContent)

find_package(re2 REQUIRED)
find_package(CLI11 REQUIRED)
find_package(nlohmann_json REQUIRED)

FetchContent_Declare(
  Lines
  GIT_REPOSITORY https://github.com/ihateyouall-dev/lines-core
  GIT_TAG main)

FetchContent_MakeAvailable(Lines)

add_library(cli_root)
add_library(tasks_json_storage)
add_library(storages_utils)
add_library(cli_tasks)
add_library(cli_docs)
add_library(client_utils)

target_sources(cli_root PRIVATE cli/root.cpp)
target_sources(cli_tasks PRIVATE cli/tasks/tasks.cpp cli/tasks/filter.cpp)
target_sources(cli_docs PRIVATE cli/docs/docs.cpp)
target_sources(tasks_json_storage PRIVATE storages/tasks/json.cpp)

file(GLOB STORAGES_UTILS_SOURCES "storages/utils/*.cpp")
file(GLOB CLIENT_UTILS_SOURCES "client-utils/*.cpp")

target_sources(storages_utils PRIVATE ${STORAGES_UTILS_SOURCES})
target_sources(client_utils PRIVATE ${CLIENT_UTILS_SOURCES})

set(LINES_SRC ${PROJECT_SOURCE_DIR}/src)

foreach(target cli_root cli_tasks cli_docs tasks_json_storage storages_utils
               client_utils)
  target_include_directories(${target} PUBLIC ${LINES_SRC})
endforeach()

target_link_libraries(cli_tasks PUBLIC CLI11::CLI11 Lines::Tasks
                                       tasks_json_storage client_utils)
target_link_libraries(cli_root PUBLIC CLI11::CLI11 Lines::Lines cli_tasks
                                      cli_docs)
target_link_libraries(cli_docs PUBLIC CLI11::CLI11)
target_link_libraries(
  tasks_json_storage PUBLIC nlohmann_json::nlohmann_json Lines::Tasks
                            storages_utils client_utils)
target_link_libraries(client_utils PUBLIC Lines::Temporal Lines::Tasks re2::re2)

add_library(Lines::CLI::Root ALIAS cli_root)
add_library(Lines::Storages::Tasks::Json ALIAS tasks_json_storage)
add_library(Lines::Storages::Utils ALIAS storages_utils)
