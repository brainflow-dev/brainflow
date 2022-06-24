cmake_minimum_required(VERSION 3.16)
project(simpleble_test)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Nice hack to automatically ignore the build directory
file(WRITE ${CMAKE_BINARY_DIR}/.gitignore "*")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include SimpleBluez
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_BINARY_DIR}/simpleble)
include_directories(${SIMPLEBLE_INCLUDES})
include_directories(${SIMPLEBLE_C_INCLUDES})

find_package(GTest REQUIRED)

set(
  SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.cpp
)

add_executable(simpleble_test ${SRC_FILES})
target_link_libraries(simpleble_test simpleble-static ${GTEST_LIBRARIES})
target_include_directories(simpleble_test PRIVATE ${GTEST_INCLUDE_DIRS})
