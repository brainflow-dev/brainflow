cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_NOTIFY_C)

message("-- [INFO] Building Example")
add_executable(example_notify_c notify_c.c)
target_link_libraries(example_notify_c simpleble-c-static)
