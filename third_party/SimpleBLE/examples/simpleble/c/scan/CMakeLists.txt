cmake_minimum_required(VERSION 3.21)

project(EXAMPLE_SCAN_C)

message("-- [INFO] Building Example")
add_executable(example_scan_c scan.c)
target_link_libraries(example_scan_c simpleble::simpleble-c)
