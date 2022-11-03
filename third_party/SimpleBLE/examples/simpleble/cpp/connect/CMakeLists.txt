cmake_minimum_required(VERSION 3.21)

project(EXAMPLE_CONNECT)

message("-- [INFO] Building Example")

add_executable(example_connect
    connect.cpp
    ../common/utils.cpp
)

target_link_libraries(example_connect simpleble::simpleble)
