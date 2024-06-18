cmake_minimum_required(VERSION 3.21)

project(EXAMPLE_READ)

message("-- [INFO] Building Example")

add_executable(example_read
    read.cpp
    ../common/utils.cpp
)

target_link_libraries(example_read simpleble::simpleble)
