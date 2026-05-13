cmake_minimum_required(VERSION 3.14)

cmake_policy(SET CMP0091 NEW)

project(lines-cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LINES_ENABLE_TESTING "Build tests" OFF)

if(MSVC)
  # MSVC historically reports __cplusplus as 199711L, so forcing it to report
  # correct value
  add_compile_options(/Zc:__cplusplus)
endif()

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} PRIVATE Lines::CLI::Root)

if(LINES_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()
