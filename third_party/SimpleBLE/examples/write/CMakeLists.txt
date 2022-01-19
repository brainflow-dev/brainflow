cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_WRITE)

message("-- [INFO] Building Example")
add_executable(example_write write.cpp)
target_link_libraries(example_write simpleble-static)
