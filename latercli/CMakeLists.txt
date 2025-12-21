add_library(latercli STATIC ../src/latercli/latercli.cpp)

target_include_directories(latercli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(include/storage)

target_link_libraries(latercli PUBLIC Later::Tasks Later::Tasks::Storage)

add_library(Later::CLI ALIAS latercli)
