cmake_minimum_required(VERSION 3.23)

cmake_policy(SET CMP0091 NEW)

project(
  lines-cli
  LANGUAGES CXX
  VERSION 0.1.0)

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

include(GNUInstallDirs)

install(TARGETS lines-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

if(WIN32)
  set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
  set(CPACK_SYSTEM_NAME "macOS")
  set(CPACK_GENERATOR "TGZ;ZIP")
else()
  set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_GENERATOR "TGZ;ZIP")
endif()

set(CPACK_SOURCE_IGNORE_FILES "/build/" "/.git/" "/.github/")

include(CPack)
