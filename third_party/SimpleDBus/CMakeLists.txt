cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SIMPLEDBUS)

# Load dependencies
find_package(PkgConfig REQUIRED)
pkg_search_module(DBUS REQUIRED dbus-1)

# Detect if the project is being build within a project or standalone.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(STANDALONE true)

    # Configure the build path
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    set(STANDALONE false)
endif()

# Load default parameters passed in through the command line.
if(NOT SIMPLEDBUS_LOG_LEVEL)
    set(SIMPLEDBUS_LOG_LEVEL "FATAL")
endif()

# Library includes
set(
    SIMPLEDBUS_ALL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${DBUS_INCLUDE_DIRS}
)

# OUTPUTS
message("-- [INFO] Building SimpleDBus")
file(GLOB_RECURSE SRC_SIMPLEDBUS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(simpledbus-static STATIC ${SRC_SIMPLEDBUS_FILES})
add_library(simpledbus SHARED ${SRC_SIMPLEDBUS_FILES})

target_compile_definitions(simpledbus-static PRIVATE SIMPLEDBUS_LOG_LEVEL=${SIMPLEDBUS_LOG_LEVEL})
target_compile_definitions(simpledbus PRIVATE SIMPLEDBUS_LOG_LEVEL=${SIMPLEDBUS_LOG_LEVEL})

target_include_directories(simpledbus-static PRIVATE ${SIMPLEDBUS_ALL_INCLUDE_DIRS})
target_include_directories(simpledbus PRIVATE ${SIMPLEDBUS_ALL_INCLUDE_DIRS})

set_property(TARGET simpledbus-static PROPERTY CXX_STANDARD 17)
set_property(TARGET simpledbus PROPERTY CXX_STANDARD 17)
set_property(TARGET simpledbus-static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET simpledbus PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_options(simpledbus-static PRIVATE -Wfatal-errors -Wpedantic -O3)
target_compile_options(simpledbus PRIVATE -Wfatal-errors -Wpedantic -O3)

target_link_libraries(simpledbus-static PUBLIC ${DBUS_LIBRARIES})
target_link_libraries(simpledbus PUBLIC ${DBUS_LIBRARIES})

# Export the variables needed by the parent project
if(NOT ${STANDALONE})
    set(SIMPLEDBUS_INCLUDES ${SIMPLEDBUS_ALL_INCLUDE_DIRS} PARENT_SCOPE)
endif()
