cmake_minimum_required(VERSION 3.16.0)

project(SimpleBLE_Examples)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include SimpleBLE
# Build artifacts in a separate folder
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_BINARY_DIR}/simpleble)
include_directories(${SIMPLEBLE_INCLUDES})

add_subdirectory(list_adapters)
add_subdirectory(scan)
add_subdirectory(scan_c)
add_subdirectory(connect)
add_subdirectory(connect_c)
add_subdirectory(connect_safe)
add_subdirectory(read)
add_subdirectory(write)
add_subdirectory(notify)
