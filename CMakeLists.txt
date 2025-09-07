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