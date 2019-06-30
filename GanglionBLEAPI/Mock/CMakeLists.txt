cmake_minimum_required (VERSION 3.10)
project (GanglionEmulator)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (LIB_NAME "GanglionLibNative64")
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (LIB_NAME "GanglionLibNative32")
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

add_library (
    ${LIB_NAME} SHARED
    GanglionNativeMock.cpp
)

target_include_directories (
    ${LIB_NAME} PRIVATE
    "${CMAKE_HOME_DIRECTORY}/../Interface"
)

set_target_properties (
    ${LIB_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
)
