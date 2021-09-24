cmake_minimum_required(VERSION 3.10)

set(project_name SecureSideEffectSearch)
project(${project_name})

find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "-O3 -std=c++11 -pthread -Wall -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -std=c++11 -pthread -Wall")

set(HELIB_DIR /usr/local/src/HElib)

include_directories(
  ${PROJECT_SOURCE_DIR}/sses
  ${PROJECT_SOURCE_DIR}/stdsc
  ${HELIB_DIR}/src
)

link_directories(${HELIB_DIR}/src)

set(COMMON_LIBS
  stdsc
  sses_share
  helib
  ntl
  gmp
)

add_subdirectory(stdsc)
add_subdirectory(sses)
add_subdirectory(demo)
