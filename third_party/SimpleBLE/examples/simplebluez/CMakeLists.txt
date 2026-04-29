cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/prelude.cmake)
project(simplebluez_examples)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIMPLEBLUEZ_LOCAL "Use local SimpleBluez" ON)

if (SIMPLEBLUEZ_LOCAL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../simplebluez ${CMAKE_BINARY_DIR}/simplebluez)
else()
    find_package(simplebluez CONFIG REQUIRED)
endif()

add_compile_definitions(FMT_HEADER_ONLY)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/internal/include)

# Macro to apply common settings to SimpleBluez example targets
macro(configure_simplebluez_target target_name)
    target_link_libraries(${target_name} simplebluez::simplebluez)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

# C++ Examples
add_executable(simplebluez_list_adapters src/list_adapters.cpp)
configure_simplebluez_target(simplebluez_list_adapters)

add_executable(simplebluez_list_paired src/list_paired.cpp)
configure_simplebluez_target(simplebluez_list_paired)

add_executable(simplebluez_scan src/scan.cpp)
configure_simplebluez_target(simplebluez_scan)

add_executable(simplebluez_connect src/connect.cpp)
configure_simplebluez_target(simplebluez_connect)

add_executable(simplebluez_pair src/pair.cpp)
configure_simplebluez_target(simplebluez_pair)

add_executable(simplebluez_read src/read.cpp)
configure_simplebluez_target(simplebluez_read)

add_executable(simplebluez_notify src/notify.cpp)
configure_simplebluez_target(simplebluez_notify)

add_executable(simplebluez_ble_nus src/ble_nus.cpp)
configure_simplebluez_target(simplebluez_ble_nus)

add_executable(simplebluez_peripheral_advertise src/peripheral_advertise.cpp)
configure_simplebluez_target(simplebluez_peripheral_advertise)

add_executable(simplebluez_peripheral_stream src/peripheral_stream.cpp)
configure_simplebluez_target(simplebluez_peripheral_stream)
