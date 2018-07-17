cmake_minimum_required (VERSION 3.10)
project (brainflow)

set (CMAKE_CXX_STANDARD 11)

include_directories (
    inc
    src/Utils/inc
)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/brainflow/lib)

add_library (
    DataReader SHARED
    src/Utils/DataBuffer.cpp
)
