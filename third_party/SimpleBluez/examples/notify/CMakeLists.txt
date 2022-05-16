cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_NOTIFY)

message("-- [INFO] Building Example")
add_executable(example_notify notify.cpp)
target_link_libraries(example_notify simplebluez-static)
