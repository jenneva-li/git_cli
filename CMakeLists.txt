cmake_minimum_required(VERSION 3.28)

project(
  GIT_CLI
  VERSION 1.0.0
  DESCRIPTION "Git Command Line Interface"
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
file(GLOB SRC_FILES "src/*.cpp")

add_executable(git_cli main.cpp ${SRC_FILES})

find_package(ZLIB REQUIRED)
target_link_libraries(git_cli PRIVATE ZLIB::ZLIB)

install(TARGETS git_cli RUNTIME DESTINATION bin)

# -------------------------
# Tests (optional)
# -------------------------
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/heads/main.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    file(GLOB TEST_FILES "tests/*.cpp")

    add_executable(git_cli_tests ${TEST_FILES} ${SRC_FILES})
    target_include_directories(git_cli_tests PRIVATE include)
    target_link_libraries(git_cli_tests PRIVATE gtest_main ZLIB::ZLIB)

    include(GoogleTest)
    gtest_discover_tests(git_cli_tests)
endif()