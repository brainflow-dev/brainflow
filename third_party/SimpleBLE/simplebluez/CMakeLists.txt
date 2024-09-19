cmake_minimum_required(VERSION 3.21)

# Only allow Linux builds
if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "-- [ERROR] UNSUPPORTED SYSTEM: '${CMAKE_HOST_SYSTEM_NAME}'. SimpleBluez can only be built on Linux.")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)

project(
    simplebluez
    VERSION ${SIMPLEBLE_VERSION}
    DESCRIPTION "A simple C++ wrapper around Bluez with a commercially-friendly licence."
    HOMEPAGE_URL "https://github.com/OpenBluetoothToolbox/SimpleBluez"
    LANGUAGES CXX
)

# Run prelude script to set up environment
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/epilogue.cmake)

option(LIBFMT_VENDORIZE "Enable vendorized libfmt" ON)

find_package(fmt REQUIRED)
set_target_properties(fmt PROPERTIES EXCLUDE_FROM_ALL TRUE)

find_package(DBus1 REQUIRED)

if(NOT DEFINED SIMPLEDBUS_SANITIZE AND DEFINED SIMPLEBLUEZ_SANITIZE)
    set(SIMPLEDBUS_SANITIZE ${SIMPLEBLUEZ_SANITIZE})
endif()

# Load all variables that would eventually need to be exposed to downstream projects
if(NOT SIMPLEDBUS_LOG_LEVEL)
    set(SIMPLEDBUS_LOG_LEVEL "FATAL")
endif()

if(NOT SIMPLEBLUEZ_LOG_LEVEL)
    set(SIMPLEBLUEZ_LOG_LEVEL "FATAL")
endif()

set(SIMPLEBLUEZ_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/include)

set(SIMPLEBLUEZ_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ProxyOrg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Agent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Characteristic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ProxyOrgBluez.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bluez.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Descriptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/Adapter1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/Agent1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/GattDescriptor1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/GattCharacteristic1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/GattService1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/Device1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/Battery1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces/AgentManager1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/advanced/Interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/advanced/Proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/base/Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/base/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/base/Holder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/base/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/base/Message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/base/Path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/src/interfaces/ObjectManager.cpp
)

# Configure the build targets

add_library(simplebluez ${SIMPLEBLUEZ_SRC})
add_library(simplebluez::simplebluez ALIAS simplebluez)

set_target_properties(
    simplebluez PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME simplebluez
    OUTPUT_NAME simplebluez
)

list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEDBUS_LOG_LEVEL=${SIMPLEDBUS_LOG_LEVEL})
list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEBLUEZ_LOG_LEVEL=${SIMPLEBLUEZ_LOG_LEVEL})

if(SIMPLEBLUEZ_USE_SESSION_DBUS)
    list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEBLUEZ_USE_SESSION_DBUS)
endif()

target_link_libraries(simplebluez PUBLIC ${DBus1_LIBRARIES})
target_link_libraries(simplebluez PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

target_include_directories(simplebluez PRIVATE ${SIMPLEBLUEZ_INCLUDE})
target_include_directories(simplebluez INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../external/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${DBus1_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${DBus1_ARCH_INCLUDE_DIR}>)

append_sanitize_options("${SIMPLEBLUEZ_SANITIZE}")

apply_build_options(simplebluez
    "${PRIVATE_COMPILE_DEFINITIONS}"
    "${PRIVATE_COMPILE_OPTIONS}"
    "${PRIVATE_LINK_OPTIONS}"
    "${PUBLIC_LINK_OPTIONS}")

include(GNUInstallDirs)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/simplebluez.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/simplebluez.pc @ONLY)

install(
    TARGETS simplebluez
    EXPORT simplebluez-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
    EXPORT simplebluez-config
    NAMESPACE simplebluez::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simplebluez)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/simplebluez/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplebluez)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../external/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplebluez)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/include/simpledbus/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simpledbus)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/simplebluez.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(SIMPLEBLUEZ_TEST)
    find_package(Python3 COMPONENTS Development REQUIRED)
    find_package(GTest REQUIRED)

    add_executable(simplebluez_test
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/helpers/PythonRunner.cpp)

    set_target_properties(simplebluez_test PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        CXX_STANDARD 17
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

    target_link_libraries(simplebluez_test PRIVATE simplebluez::simplebluez ${GTEST_LIBRARIES} ${Python3_LIBRARIES} pthread)
    target_include_directories(simplebluez_test PRIVATE ${GTEST_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})

    add_custom_command (TARGET simplebluez_test POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/python/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
