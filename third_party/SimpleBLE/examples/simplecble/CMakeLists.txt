cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/prelude.cmake)
project(simplecble_examples)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIMPLEBLE_LOCAL "Use local SimpleBLE" ON)

if (SIMPLEBLE_LOCAL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../simplecble ${CMAKE_BINARY_DIR}/simplecble)
else()
    cmake_policy(SET CMP0144 OLD)
    find_package(simplecble CONFIG REQUIRED)
endif()

# Macro to apply common settings to SimpleCBLE example targets
macro(configure_simplecble_target target_name)
    target_link_libraries(${target_name} simplecble::simplecble)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

# C Examples
add_executable(simplecble_scan src/scan.c)
configure_simplecble_target(simplecble_scan)

add_executable(simplecble_connect src/connect.c)
configure_simplecble_target(simplecble_connect)

add_executable(simplecble_notify src/notify.c)
configure_simplecble_target(simplecble_notify)