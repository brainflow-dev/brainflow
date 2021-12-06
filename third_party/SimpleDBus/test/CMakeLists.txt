cmake_minimum_required(VERSION 3.16)
project(simpledbus_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include SimpleDBus
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_BINARY_DIR}/simpledbus)
include_directories(${SIMPLEDBUS_INCLUDES})

find_package(Python3 COMPONENTS Development REQUIRED)

enable_testing()
find_package(GTest REQUIRED)

set(
  SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test_holder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test_message.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/PythonRunner.cpp
)

add_executable(simpledbus_test ${SRC_FILES})
target_link_libraries(simpledbus_test simpledbus-static ${GTEST_LIBRARIES} ${Python3_LIBRARIES} pthread)
target_include_directories(simpledbus_test PRIVATE ${GTEST_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})

add_custom_command (TARGET simpledbus_test POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/python/ ${CMAKE_BINARY_DIR}
)
