cmake_minimum_required(VERSION 3.21)

# Only allow Linux builds
if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "-- [ERROR] UNSUPPORTED SYSTEM: '${CMAKE_HOST_SYSTEM_NAME}'. SimpleDBus can only be built on Linux.")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)

project(
    simpledbus
    VERSION ${SIMPLEBLE_VERSION}
    DESCRIPTION "A simple C++ wrapper around DBus with a commercially-friendly licence."
    HOMEPAGE_URL "https://github.com/OpenBluetoothToolbox/SimpleDBus"
    LANGUAGES CXX
)

# Run prelude script to set up environment
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/epilogue.cmake)

option(LIBFMT_VENDORIZE "Enable vendorized libfmt" ON)

find_package(fmt REQUIRED)
set_target_properties(fmt PROPERTIES EXCLUDE_FROM_ALL TRUE)

find_package(DBus1 REQUIRED)

# Load all variables that would eventually need to be exposed to downstream projects
if(NOT SIMPLEDBUS_LOG_LEVEL)
    set(SIMPLEDBUS_LOG_LEVEL "FATAL")
endif()

set(SIMPLEDBUS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SIMPLEDBUS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/advanced/Interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/advanced/Proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Holder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/ObjectManager.cpp)

# Configure the build targets

add_library(simpledbus ${SIMPLEDBUS_SRC})
add_library(simpledbus::simpledbus ALIAS simpledbus)

set_target_properties(
    simpledbus PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME simpledbus
    OUTPUT_NAME simpledbus
)

list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEDBUS_LOG_LEVEL=${SIMPLEDBUS_LOG_LEVEL})

target_link_libraries(simpledbus PUBLIC ${DBus1_LIBRARIES})
target_link_libraries(simpledbus PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

target_include_directories(simpledbus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(simpledbus INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${DBus1_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${DBus1_ARCH_INCLUDE_DIR}>)

append_sanitize_options("${SIMPLEDBUS_SANITIZE}")

apply_build_options(simpledbus
    "${PRIVATE_COMPILE_DEFINITIONS}"
    "${PRIVATE_COMPILE_OPTIONS}"
    "${PRIVATE_LINK_OPTIONS}"
    "${PUBLIC_LINK_OPTIONS}")

include(GNUInstallDirs)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/simpledbus.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/simpledbus.pc @ONLY)

install(
    TARGETS simpledbus
    EXPORT simpledbus-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
    EXPORT simpledbus-config
    NAMESPACE simpledbus::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simpledbus)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/simpledbus/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simpledbus)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/simpledbus.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)


if(SIMPLEDBUS_TEST)
    find_package(Python3 COMPONENTS Development REQUIRED)
    find_package(GTest REQUIRED)

    add_executable(simpledbus_test
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_holder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_proxy_interfaces.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_proxy_children.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_proxy_lifetime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_path.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/helpers/PythonRunner.cpp)

    set_target_properties(simpledbus_test PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        CXX_STANDARD 17
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

    target_link_libraries(simpledbus_test PRIVATE simpledbus::simpledbus ${GTEST_LIBRARIES} ${Python3_LIBRARIES} pthread)
    target_include_directories(simpledbus_test PRIVATE ${GTEST_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})

    add_custom_command (TARGET simpledbus_test POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/python/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
