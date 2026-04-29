cmake_minimum_required(VERSION 3.21)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Common configuration used by all projects in this repository
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/find)

include(${CMAKE_CURRENT_LIST_DIR}/parse_version.cmake)
parse_version(${CMAKE_CURRENT_LIST_DIR}/../VERSION SIMPLEBLE)
