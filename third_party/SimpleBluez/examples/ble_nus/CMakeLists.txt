cmake_minimum_required(VERSION 3.16.0)

project(EXAMPLE_BLE_NUS)

message("-- [INFO] Building Example")
add_executable(example_ble_nus ble_nus.cpp)
target_link_libraries(example_ble_nus simplebluez-static)
