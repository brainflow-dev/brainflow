cmake_minimum_required (VERSION 3.10)
project (brainflow_cpp_example)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

find_package (
    brainflow CONFIG REQUIRED
)

add_executable (
    test_get_data
    src/brainflow_get_data.cpp
)

target_include_directories (
    test_get_data PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    test_get_data PUBLIC
    ${GetDataPath}
    ${BoardControllerPath}
)
