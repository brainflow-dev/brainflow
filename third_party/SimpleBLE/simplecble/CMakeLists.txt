cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)

project(
    simplecble
    VERSION ${SIMPLEBLE_VERSION_BASE}
    DESCRIPTION "The ultimate fully-fledged cross-platform library for Bluetooth Low Energy (BLE)."
    HOMEPAGE_URL "https://github.com/simpleble/simpleble"
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/epilogue.cmake)

include(GenerateExportHeader)
include(GNUInstallDirs)

if(APPLE)
    set(SIMPLECBLE_OUTPUT_NAME "simpleble-c")
    set(SIMPLECBLE_FILE_NAME "libsimpleble-c.dylib")
elseif(UNIX)
    set(SIMPLECBLE_OUTPUT_NAME "simpleble-c")
    set(SIMPLECBLE_FILE_NAME "libsimpleble-c.so")
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SIMPLECBLE_OUTPUT_NAME "simpleble-c")
        set(SIMPLECBLE_FILE_NAME "simpleble-c.dll")
    else()
        set(SIMPLECBLE_OUTPUT_NAME "simpleble-c32")
        set(SIMPLECBLE_FILE_NAME "simpleble-c32.dll")
    endif()
endif()

if(SIMPLECBLE_TEST)
    message(STATUS "Building tests requires plain version of SimpleBLE")
    set(SIMPLEBLE_PLAIN ON)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../simpleble ${CMAKE_BINARY_DIR}/simpleble)

set(SIMPLEBLE_C_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simplecble.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peripheral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp)

add_library(simplecble SHARED ${SIMPLEBLE_C_SRC})
add_library(simplecble::simplecble ALIAS simplecble)

# NOTE: This is a temporary alias to ensure backwards compatibility with the old C bindings.
add_library(simpleble-c ALIAS simplecble)
add_library(simpleble::simpleble-c ALIAS simplecble)

set_target_properties(simplecble PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    DEFINE_SYMBOL simpleble_EXPORTS
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME simplecble
    OUTPUT_NAME ${SIMPLECBLE_OUTPUT_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled
    RELEASE_POSTFIX ""
    RELWITHDEBINFO_POSTFIX "-relwithdebinfo"
    MINSIZEREL_POSTFIX "-minsizerel"
    DEBUG_POSTFIX "")

# TODO: We need to migrate everything internally to use the new export header.
generate_export_header(
    simplecble
    BASE_NAME simplecble
    EXPORT_FILE_NAME export/simplecble/export.h
)

# Configure include directories for simplecble
target_include_directories(simplecble PRIVATE ${SIMPLEBLE_PRIVATE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(simplecble INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(simplecble SYSTEM PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>)

# Configure linked libraries for simplecble
target_link_libraries(simplecble PRIVATE simpleble::simpleble)
target_compile_definitions(simplecble PRIVATE SIMPLEBLE_VERSION="${PROJECT_VERSION}")

# Apply OS-specific settings for simplecble
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_property(TARGET simplecble PROPERTY INSTALL_RPATH $ORIGIN)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_property(TARGET simplecble PROPERTY INSTALL_RPATH @loader_path)
endif()

apply_build_options(simplecble
    "${PRIVATE_COMPILE_DEFINITIONS}"
    "${PRIVATE_COMPILE_OPTIONS}"
    "${PRIVATE_LINK_OPTIONS}"
    "${PUBLIC_LINK_OPTIONS}")

install(
    TARGETS simplecble
    EXPORT simplecble-config
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib)

if(MSVC)
    add_custom_command(TARGET simplecble POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../python_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../java_package/brainflow/src/main/resources/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../csharp_package/brainflow/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../matlab_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../julia_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../rust_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../nodejs_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
    )
endif()
if(UNIX AND NOT ANDROID)
    add_custom_command(TARGET simplecble POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../python_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../julia_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../java_package/brainflow/src/main/resources/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../csharp_package/brainflow/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../matlab_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../rust_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${SIMPLECBLE_FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../nodejs_package/brainflow/lib/${SIMPLECBLE_FILE_NAME}"
    )
endif()

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/simplecble/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplecble)

if(SIMPLECBLE_TEST)
    message(STATUS "No tests are available for simplecble")
    # find_package(GTest REQUIRED)

    # add_executable(simpleble_test
    #     ${CMAKE_CURRENT_SOURCE_DIR}/test/src/main.cpp
    #     ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_utils.cpp
    #     ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_bytearray.cpp)
    # set_target_properties(simpleble_test PROPERTIES
    #     CXX_VISIBILITY_PRESET hidden
    #     VISIBILITY_INLINES_HIDDEN YES
    #     CXX_STANDARD 17
    #     POSITION_INDEPENDENT_CODE ON
    #     WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # target_link_libraries(simpleble_test PRIVATE simpleble::simpleble GTest::gtest)
endif()

