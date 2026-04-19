cmake_minimum_required(VERSION 3.14)
project(lines-cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE Lines::CLI::Root)

enable_testing()
add_subdirectory(tests)
