cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/prelude.cmake)
project(simpleble_examples)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIMPLEBLE_LOCAL "Use local SimpleBLE" ON)

if (SIMPLEBLE_LOCAL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../simpleble ${CMAKE_BINARY_DIR}/simpleble)
else()
    find_package(simpleble CONFIG REQUIRED)
endif()

# C++
add_subdirectory(cpp/list_adapters)
add_subdirectory(cpp/scan)
add_subdirectory(cpp/connect)
add_subdirectory(cpp/connect_safe)
add_subdirectory(cpp/read)
add_subdirectory(cpp/write)
add_subdirectory(cpp/notify)
add_subdirectory(cpp/notify_multi)

# C
add_subdirectory(c/notify)
add_subdirectory(c/connect)
add_subdirectory(c/scan)
