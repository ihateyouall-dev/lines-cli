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

FetchContent_MakeAvailable(CLI11)
FetchContent_MakeAvailable(Lines)

add_library(cliroot)

target_sources(cliroot PRIVATE cli/root.cpp)

target_include_directories(cliroot PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(cliroot PUBLIC CLI11 Lines::Lines)

add_library(Lines::CLI::Root ALIAS cliroot)
