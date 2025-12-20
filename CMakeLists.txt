cmake_minimum_required(VERSION 3.28)
project(later)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(latercli)

target_link_libraries(${PROJECT_NAME} PRIVATE Later::CLI)
