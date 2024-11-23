cmake_minimum_required(VERSION 3.21)

project(EXAMPLE_LIST_ADAPTERS_SAFE)

message("-- [INFO] Building Example")

add_executable(example_list_adapters_safe
    list_adapters_safe.cpp
)

target_link_libraries(example_list_adapters_safe simpleble::simpleble)
