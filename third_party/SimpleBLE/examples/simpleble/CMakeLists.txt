cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/prelude.cmake)
project(simpleble_examples)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIMPLEBLE_LOCAL "Use local SimpleBLE" ON)

if (SIMPLEBLE_LOCAL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../simpleble ${CMAKE_BINARY_DIR}/simpleble)
else()
    cmake_policy(SET CMP0144 OLD)
    find_package(simpleble CONFIG REQUIRED)
endif()

# Macro to apply common settings to SimpleBLE example targets
macro(configure_simpleble_target target_name)
    target_link_libraries(${target_name} simpleble::simpleble)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

# C++ Examples
add_executable(simpleble_list_adapters src/list_adapters.cpp)
configure_simpleble_target(simpleble_list_adapters)

add_executable(simpleble_power_cycle src/power_cycle.cpp)
configure_simpleble_target(simpleble_power_cycle)

add_executable(simpleble_list_paired src/list_paired.cpp)
configure_simpleble_target(simpleble_list_paired)

add_executable(simpleble_list_adapters_safe src/list_adapters_safe.cpp)
configure_simpleble_target(simpleble_list_adapters_safe)

add_executable(simpleble_scan src/scan.cpp src/utils.cpp)
configure_simpleble_target(simpleble_scan)

add_executable(simpleble_connect src/connect.cpp src/utils.cpp)
configure_simpleble_target(simpleble_connect)

add_executable(simpleble_connect_safe src/connect_safe.cpp src/utils.cpp)
configure_simpleble_target(simpleble_connect_safe)

add_executable(simpleble_multiconnect src/multiconnect.cpp src/utils.cpp)
configure_simpleble_target(simpleble_multiconnect)

add_executable(simpleble_read src/read.cpp src/utils.cpp)
configure_simpleble_target(simpleble_read)

add_executable(simpleble_write src/write.cpp src/utils.cpp)
configure_simpleble_target(simpleble_write)

add_executable(simpleble_notify src/notify.cpp src/utils.cpp)
configure_simpleble_target(simpleble_notify)

add_executable(simpleble_indicate src/indicate.cpp src/utils.cpp)
configure_simpleble_target(simpleble_indicate)

add_executable(simpleble_notify_multi src/notify_multi.cpp src/utils.cpp)
configure_simpleble_target(simpleble_notify_multi)