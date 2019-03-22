cmake_minimum_required (VERSION 3.10)
project (brainflow_cpp_example)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

include_directories (
    ${CMAKE_HOME_DIRECTORY}/../../data/inc
)

find_library (
    brainflow
    BoardController
    HINTS "${CMAKE_HOME_DIRECTORY}/../../data/lib"
)

add_executable (
    GetData
    src/board_shim.cpp
    src/data_handler.cpp
    src/brainflow_get_data.cpp
)

target_link_libraries (GetData "${brainflow}")

if (UNIX)
    target_link_libraries (GetData pthread)
endif (UNIX)
