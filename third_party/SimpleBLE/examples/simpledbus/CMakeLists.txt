cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/prelude.cmake)
project(simpledbus_examples)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIMPLEDBUS_LOCAL "Use local SimpleDBus" ON)

if (SIMPLEDBUS_LOCAL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../simpledbus ${CMAKE_BINARY_DIR}/simpledbus)
else()
    find_package(simpledbus CONFIG REQUIRED)
endif()

add_subdirectory(notification)
