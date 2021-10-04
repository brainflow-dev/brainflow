cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_CONNECT_C)

message("-- [INFO] Building Example")
add_executable(example_connect_c connect_c.c)
target_link_libraries(example_connect_c simpleble-c-static)
