cmake_minimum_required (VERSION 3.13)
project (serial_discovery_tests LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component (BRAINFLOW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
find_package (Threads REQUIRED)
add_executable (serial_discovery_tests
    serial_discovery_tests.cpp
    ${BRAINFLOW_ROOT}/src/utils/serial_port_info.cpp
    ${BRAINFLOW_ROOT}/src/board_controller/openbci/openbci_serial_helpers.cpp)
target_include_directories (serial_discovery_tests PRIVATE
    ${BRAINFLOW_ROOT}/src/utils/inc
    ${BRAINFLOW_ROOT}/src/board_controller/openbci/inc)
target_compile_definitions (serial_discovery_tests PRIVATE NOMINMAX)
target_link_libraries (serial_discovery_tests PRIVATE Threads::Threads)
include (${BRAINFLOW_ROOT}/cmake/serial_port_info.cmake)
link_serial_port_info (serial_discovery_tests)
if (MSVC)
    target_compile_options (serial_discovery_tests PRIVATE /W4 /WX)
else ()
    target_compile_options (serial_discovery_tests PRIVATE -Wall -Wextra -Werror)
endif ()
enable_testing ()
add_test (NAME serial_discovery_tests COMMAND serial_discovery_tests)
