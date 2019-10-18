cmake_minimum_required(VERSION 3.0)

project (DSPFilters CXX)

aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR}/source SOURCE_LIB)

add_library (DSPFilters STATIC ${SOURCE_LIB})

target_include_directories (DSPFilters PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set_property (TARGET DSPFilters PROPERTY POSITION_INDEPENDENT_CODE ON)
