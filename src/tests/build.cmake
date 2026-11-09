SET (TESTS_EXE_NAME "brainflow_tests")

# Show Google Test options
MARK_AS_ADVANCED (CLEAR BUILD_GMOCK)
MARK_AS_ADVANCED (CLEAR INSTALL_GTEST)
MARK_AS_ADVANCED (CLEAR FETCHCONTENT_BASE_DIR)
MARK_AS_ADVANCED (CLEAR FETCHCONTENT_FULLY_DISCONNECTED)
MARK_AS_ADVANCED (CLEAR FETCHCONTENT_QUIET)
MARK_AS_ADVANCED (CLEAR FETCHCONTENT_SOURCE_DIR_GOOGLETEST)
MARK_AS_ADVANCED (CLEAR FETCHCONTENT_UPDATES_DISCONNECTED)
MARK_AS_ADVANCED (CLEAR FETCHCONTENT_UPDATES_DISCONNECTED_GOOGLETEST)

SET (BUILD_GMOCK ON CACHE BOOL "" FORCE)
SET (INSTALL_GTEST ON CACHE BOOL "" FORCE)
SET (FETCHCONTENT_QUIET ON CACHE BOOL "" FORCE)

# For Windows: Prevent overriding compiler/linker settings
SET (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(googletest)

enable_testing()

SET (TESTS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/brainflow_boards.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ble_lib_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/brainalive/brainalive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/polar/polar_h10.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/file_streamer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/multicast_streamer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/plotjuggler_udp_streamer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/bluetooth_functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/data_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/multicast_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/socket_client_udp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/timestamp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/board_controller/polar_h10_unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/board_controller/brainalive_unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/bluetooth/socket_bluetooth_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/bluetooth/bluetooth_functions_unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/custom_cast_unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/data_buffer_unittest.cpp
)

add_executable(
    ${TESTS_EXE_NAME}
    ${TESTS_SRC}
)

get_target_property (BOARD_TEST_INCLUDE_DIRS ${BOARD_CONTROLLER_NAME} INCLUDE_DIRECTORIES)
target_include_directories (
    ${TESTS_EXE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/bluetooth/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/macos_third_party
    ${BOARD_TEST_INCLUDE_DIRS}
)

target_compile_definitions (${TESTS_EXE_NAME} PRIVATE NOMINMAX)

target_link_libraries(
    ${TESTS_EXE_NAME} PRIVATE
    gmock_main
    ${CMAKE_DL_LIBS}
)

if (ANDROID)
    target_link_libraries (${TESTS_EXE_NAME} PRIVATE log)
endif (ANDROID)
if (WIN32)
    target_link_libraries (${TESTS_EXE_NAME} PRIVATE ws2_32 shlwapi)
endif (WIN32)

set_target_properties (${TESTS_EXE_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
)

include(GoogleTest)
gtest_discover_tests(${TESTS_EXE_NAME})
