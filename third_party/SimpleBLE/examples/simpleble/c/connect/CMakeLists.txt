cmake_minimum_required(VERSION 3.21)

project(EXAMPLE_CONNECT_C)

message("-- [INFO] Building Example")
add_executable(example_connect_c connect.c)
target_link_libraries(example_connect_c simpleble::simpleble-c)
