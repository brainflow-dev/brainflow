cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_LIST_ADAPTERS)

message("-- [INFO] Building Example")
add_executable(example_list_adapters main.cpp)
target_link_libraries(example_list_adapters simplebluez-static pthread)
