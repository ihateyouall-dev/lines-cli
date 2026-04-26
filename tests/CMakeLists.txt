include(FetchContent)
FetchContent_Declare(
  googletest
  URL
  "https://github.com/google/googletest/archive/
03597a01ee50ed33e9dfd640b249b4be3799d395.zip"
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB PARSERS_TESTS "parsers/*.cpp")

add_executable(parsers_tests ${PARSERS_TESTS})

target_link_libraries(parsers_tests PRIVATE GTest::gtest_main client_utils)

add_test(NAME parsers_tests COMMAND parsers_tests)
