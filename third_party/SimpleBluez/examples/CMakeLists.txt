cmake_minimum_required(VERSION 3.0.0)

project(SIMPLEBLUEZ_EXAMPLES)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include simplebluez
# Build artifacts in a separate folder
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_BINARY_DIR}/simplebluez)
include_directories(${SIMPLEBLUEZ_INCLUDES})

add_subdirectory(ble_nus)
add_subdirectory(list_adapters)
add_subdirectory(manuf_data)
