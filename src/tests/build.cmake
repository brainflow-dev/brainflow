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
    URL https://github.com/google/googletest/archive/HEAD.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

SET (TESTS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/bluetooth_functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/data_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/bluetooth/socket_bluetooth_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/bluetooth/bluetooth_functions_unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/data_buffer_unittest.cpp
)

add_executable(
    ${TESTS_EXE_NAME}
    ${TESTS_SRC}
)

target_include_directories (
    ${TESTS_EXE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/utils/bluetooth/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/macos_third_party
)

target_link_libraries(
    ${TESTS_EXE_NAME} PRIVATE
    gmock_main
)

set_target_properties (${TESTS_EXE_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
)

include(GoogleTest)
gtest_discover_tests(${TESTS_EXE_NAME})