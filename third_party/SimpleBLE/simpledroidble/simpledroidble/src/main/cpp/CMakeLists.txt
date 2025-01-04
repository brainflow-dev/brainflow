cmake_minimum_required(VERSION 3.21)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

include(${PROJECT_ROOT_DIR}/cmake/prelude.cmake)

project(
    "simpleble-jni"
    VERSION ${SIMPLEBLE_VERSION}
    LANGUAGES CXX
)

include(${PROJECT_ROOT_DIR}/cmake/epilogue.cmake)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${PROJECT_ROOT_DIR}/simpleble ${CMAKE_BINARY_DIR}/simpleble)
set(BUILD_SHARED_LIBS ON)

if(NOT TARGET fmt::fmt-header-only)
    option(LIBFMT_VENDORIZE "Enable vendorized libfmt" ON)
    find_package(fmt REQUIRED)

    if(TARGET fmt)
        set_target_properties(fmt PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endif()

add_library(
    ${CMAKE_PROJECT_NAME} SHARED
    simpleble_android.cpp
    android_utils.cpp
)

set_target_properties(
    ${CMAKE_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    android log simpleble::simpleble fmt::fmt-header-only
)