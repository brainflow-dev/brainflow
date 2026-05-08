cmake_minimum_required(VERSION 3.21)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include(${PROJECT_ROOT_DIR}/cmake/prelude.cmake)

project(
    "simplejavable"
    VERSION ${SIMPLEBLE_VERSION_BASE}
    LANGUAGES CXX
)

include(${PROJECT_ROOT_DIR}/cmake/epilogue.cmake)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${PROJECT_ROOT_DIR}/simpleble ${CMAKE_BINARY_DIR}/simpleble)
set(BUILD_SHARED_LIBS ON)

find_package(JNI REQUIRED)

add_library(
    ${CMAKE_PROJECT_NAME} SHARED
    src/core/AdapterWrapper.cpp
    src/core/Cache.cpp
    src/core/PeripheralWrapper.cpp
    src/org/simplejavable/AdapterCallback.cpp
    src/org/simplejavable/PeripheralCallback.cpp
    src/org/simplejavable/DataCallback.cpp
    src/org/simplejavable/Service.cpp
    src/org/simplejavable/Characteristic.cpp
    src/org/simplejavable/Descriptor.cpp
    src/java/lang/HashMap.cpp
    src/java/lang/Integer.cpp
    src/java/lang/Iterator.cpp
    src/java/lang/ArrayList.cpp
    src/simplejavable.cpp
    ${SIMPLEJNI_SOURCES}
)

set_target_properties(
    ${CMAKE_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    CMAKE_BUILD_TYPE Debug # TODO: Remove before release
)

target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/external
    ${CMAKE_CURRENT_SOURCE_DIR}/../../dependencies/internal/include
    ${JNI_INCLUDE_DIRS}
)

target_link_libraries(${CMAKE_PROJECT_NAME} simpleble::simpleble ${JNI_LIBRARIES})

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE FMT_HEADER_ONLY FMT_UNICODE=0)
