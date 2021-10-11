cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_MANUF_DATA)

message("-- [INFO] Building Example")
add_executable(example_manuf_data main.cpp)
target_link_libraries(example_manuf_data simplebluez-static pthread)
