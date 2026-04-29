cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

project(simplepyble)

include(GNUInstallDirs)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/epilogue.cmake)

find_package(pybind11 REQUIRED)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../simpleble ${CMAKE_BINARY_DIR}/simpleble)

pybind11_add_module(
    _simplepyble
    src/main.cpp
    src/wrap_adapter.cpp
    src/wrap_peripheral.cpp
    src/wrap_service.cpp
    src/wrap_characteristic.cpp
    src/wrap_descriptor.cpp
    src/wrap_types.cpp
    src/wrap_config.cpp
)

target_link_libraries(_simplepyble PRIVATE simpleble::simpleble)
target_compile_definitions(_simplepyble PRIVATE "-DSIMPLEPYBLE_VERSION=\"${SIMPLEPYBLE_VERSION}\"")

set_target_properties(
    _simplepyble PROPERTIES
    CXX_STANDARD 17
)

install(
    TARGETS _simplepyble
    DESTINATION .
)
