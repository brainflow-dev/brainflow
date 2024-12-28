cmake_minimum_required(VERSION 3.21)

project(EXAMPLE_MULTICONNECT)

message("-- [INFO] Building Example")

add_executable(example_multiconnect
    multiconnect.cpp
    ../common/utils.cpp
)

target_link_libraries(example_multiconnect simpleble::simpleble)
