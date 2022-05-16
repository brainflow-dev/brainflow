cmake_minimum_required(VERSION 3.0.0)

project(SIMPLEBLUEZ_EXAMPLES)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Nice hack to automatically ignore the build directory
file(WRITE ${CMAKE_BINARY_DIR}/.gitignore "*")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include simplebluez
# Build artifacts in a separate folder
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_BINARY_DIR}/simplebluez)
include_directories(${SIMPLEBLUEZ_INCLUDES})

add_subdirectory(list_adapters)
add_subdirectory(list_paired)
add_subdirectory(scan)
add_subdirectory(connect)
add_subdirectory(pair)
add_subdirectory(read)
add_subdirectory(notify)
add_subdirectory(ble_nus)
