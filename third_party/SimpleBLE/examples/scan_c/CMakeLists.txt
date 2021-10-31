cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_SCAN_C)

message("-- [INFO] Building Example")
add_executable(example_scan_c scan_c.c)
target_link_libraries(example_scan_c simpleble-c-static)
