cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)
project(simpleble_examples)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../simpleble ${CMAKE_BINARY_DIR}/simpleble)
find_package(googletest REQUIRED)

add_executable(simpleble_hitl
    src/test_sanity.cpp
)

target_link_libraries(simpleble_hitl simpleble::simpleble GTest::gtest_main)
