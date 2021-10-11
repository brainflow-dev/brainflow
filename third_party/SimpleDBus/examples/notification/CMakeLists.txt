cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_NOTIFICATION)

message("-- [INFO] Building Example")
add_executable(example_notification main.cpp)
target_link_libraries(example_notification simpledbus-static)
