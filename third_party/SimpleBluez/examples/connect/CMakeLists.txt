cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_CONNECT)

message("-- [INFO] Building Example")
add_executable(example_connect connect.cpp)
target_link_libraries(example_connect simplebluez-static)
