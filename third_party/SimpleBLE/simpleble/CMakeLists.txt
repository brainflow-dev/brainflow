cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)

project(
    simpleble
    VERSION ${SIMPLEBLE_VERSION}
    DESCRIPTION "The ultimate fully-fledged cross-platform library for Bluetooth Low Energy (BLE)."
    HOMEPAGE_URL "https://github.com/OpenBluetoothToolbox/SimpleBLE"
    LANGUAGES CXX
)

if(APPLE)
    SET(SIMPLEBLE-C simpleble-c)
    SET(FILE_NAME "libsimpleble-c.dylib")
elseif(UNIX)
    SET(SIMPLEBLE-C simpleble-c)
    SET(FILE_NAME "libsimpleble-c.so")
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(SIMPLEBLE-C simpleble-c)
        SET(FILE_NAME "simpleble-c.dll")
    else(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(SIMPLEBLE-C simpleble-c32)
        SET(FILE_NAME "simpleble-c32.dll")
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
endif(APPLE)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/epilogue.cmake)

include(GenerateExportHeader)
include(GNUInstallDirs)

option(SIMPLEBLE_PLAIN "Use plain version of SimpleBLE" OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../fmt ${CMAKE_CURRENT_BINARY_DIR}/libfmt)

if(SIMPLEBLE_TEST)
    message(STATUS "Building tests requires plain version of SimpleBLE")
    set(SIMPLEBLE_PLAIN ON)
endif()

set(SIMPLEBLE_PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builders
    ${CMAKE_CURRENT_SOURCE_DIR}/src/external
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/safe
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/include
    )

set(SIMPLEBLE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/base/Adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/base/Peripheral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/base/Service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/base/Characteristic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/base/Descriptor.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/common/ServiceBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/common/CharacteristicBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/common/DescriptorBase.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logging.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/safe/AdapterSafe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/frontends/safe/PeripheralSafe.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/builders/AdapterBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builders/PeripheralBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builders/ServiceBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builders/CharacteristicBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builders/DescriptorBuilder.cpp)

set(SIMPLEBLE_C_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/simpleble.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/peripheral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/utils.cpp)

# Define targets
add_library(simpleble ${SIMPLEBLE_SRC})

if(ANDROID)
    add_library(simpleble-c STATIC ${SIMPLEBLE_C_SRC})
else(ANDROID)
    add_library(${SIMPLEBLE-C} SHARED ${SIMPLEBLE_C_SRC})
endif(ANDROID)

add_library(simpleble::simpleble ALIAS simpleble)

set_target_properties(simpleble PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME simpleble
    OUTPUT_NAME simpleble
    RELEASE_POSTFIX ""
    RELWITHDEBINFO_POSTFIX "-relwithdebinfo"
    MINSIZEREL_POSTFIX "-minsizerel"
    DEBUG_POSTFIX "")

set_target_properties(${SIMPLEBLE-C} PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    DEFINE_SYMBOL simpleble_EXPORTS # Use the same symbol as simpleble
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled
)

# Configure include directories
target_include_directories(simpleble PRIVATE ${SIMPLEBLE_PRIVATE_INCLUDES})
target_include_directories(${SIMPLEBLE-C} PRIVATE ${SIMPLEBLE_PRIVATE_INCLUDES})

target_include_directories(simpleble INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../external/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(${SIMPLEBLE-C} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_include_directories(simpleble SYSTEM PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>)

target_include_directories(${SIMPLEBLE-C} SYSTEM PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>)

# Configure linked libraries
target_link_libraries(simpleble PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)
target_link_libraries(${SIMPLEBLE-C} PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)
target_link_libraries(${SIMPLEBLE-C} PRIVATE simpleble::simpleble)

append_sanitize_options("${SIMPLEBLE_SANITIZE}")

if(NOT SIMPLEBLE_LOG_LEVEL)
    set(SIMPLEBLE_LOG_LEVEL "INFO")
endif()

list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEBLE_LOG_LEVEL=SIMPLEBLE_LOG_LEVEL_${SIMPLEBLE_LOG_LEVEL})
list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEBLE_VERSION="${PROJECT_VERSION}")

# Detect the operating system and load the necessary dependencies
if(SIMPLEBLE_PLAIN)
    message(STATUS "Plain Flavor Requested")

    target_sources(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/plain/AdapterBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/plain/PeripheralBase.cpp)

    target_include_directories(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/plain)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Linux Host Detected")

    find_package(DBus1 REQUIRED)

    if(NOT SIMPLEDBUS_LOG_LEVEL)
        set(SIMPLEDBUS_LOG_LEVEL "FATAL")
    endif()

    if(NOT SIMPLEBLUEZ_LOG_LEVEL)
        set(SIMPLEBLUEZ_LOG_LEVEL "FATAL")
    endif()

    list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEDBUS_LOG_LEVEL=${SIMPLEDBUS_LOG_LEVEL})
    list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEBLUEZ_LOG_LEVEL=${SIMPLEBLUEZ_LOG_LEVEL})

    if(SIMPLEBLE_USE_SESSION_DBUS)
        list(APPEND PRIVATE_COMPILE_DEFINITIONS SIMPLEBLUEZ_USE_SESSION_DBUS)
    endif()

    target_sources(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/linux/AdapterBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/linux/PeripheralBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/linux/Bluez.cpp)

    target_sources(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/ProxyOrg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Logging.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Agent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Characteristic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Exceptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/ProxyOrgBluez.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Service.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Adapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Bluez.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/Descriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/Adapter1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/Agent1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/GattDescriptor1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/GattCharacteristic1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/GattService1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/Device1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/Battery1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/src/interfaces/AgentManager1.cpp
    )

    target_sources(simpleble PRIVATE
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

    target_link_libraries(simpleble PUBLIC pthread)
    target_link_libraries(simpleble PRIVATE ${DBus1_LIBRARIES})

    target_include_directories(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../simplebluez/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../simpledbus/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/linux)

    set_property(TARGET simpleble PROPERTY INSTALL_RPATH $ORIGIN)
    set_property(TARGET ${SIMPLEBLE-C} PROPERTY INSTALL_RPATH $ORIGIN)

elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Windows Host Detected")

    set(WINVERSION_CODE 0x0A00) # Selected Windows 10 based on https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Add all the special definitions for Visual Studio C++

        # /D_WIN32_WINNT -> Specifies the minimum version of Windows that the application is compatible with.
        list(APPEND PRIVATE_COMPILE_DEFINITIONS "/D_WIN32_WINNT=${WINVERSION_CODE}")
        # /D_USE_MATH_DEFINES -> Specifies that the math.h header file should be included.
        list(APPEND PRIVATE_COMPILE_DEFINITIONS "/D_USE_MATH_DEFINES")

        # /utf-8 -> Set source and executable character sets to utf-8. https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
        list(APPEND PRIVATE_COMPILE_OPTIONS "/utf-8")
        # /Gd -> Use __cdecl as the default calling convention. https://docs.microsoft.com/en-us/cpp/cpp/cdecl
        list(APPEND PRIVATE_COMPILE_OPTIONS "/Gd")
        # /EHsc -> Use the standard C++ exception handling model. https://learn.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model
        list(APPEND PRIVATE_COMPILE_OPTIONS "/EHsc")
        # /WX -> Treats all warnings as errors.
        list(APPEND PRIVATE_COMPILE_OPTIONS "/WX")
        # /W1 -> Use the lowest level of warnings, as there are some unsafe functions that MSVC doesn't like.
        # TODO: This should be removed once the warnings are fixed.
        list(APPEND PRIVATE_COMPILE_OPTIONS "/W1")
    endif()

    target_include_directories(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/windows)
    target_sources(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/windows/AdapterBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/windows/PeripheralBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/windows/Utils.cpp)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    message(STATUS "Darwin / iOS Host Detected")

    list(APPEND PRIVATE_COMPILE_OPTIONS -fobjc-arc)

    target_link_libraries(simpleble PUBLIC "-framework Foundation" "-framework CoreBluetooth" ObjC)
    target_include_directories(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/macos)
    target_sources(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/macos/Utils.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/macos/AdapterBase.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/macos/AdapterBaseMacOS.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/macos/PeripheralBase.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/macos/PeripheralBaseMacOS.mm)

    set_property(TARGET simpleble PROPERTY INSTALL_RPATH @loader_path)
    set_property(TARGET ${SIMPLEBLE-C} PROPERTY INSTALL_RPATH @loader_path)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    message(STATUS "Configuring for Android")

    # Set the include directories for the Android NDK headers and other necessary directories
    include_directories(${ANDROID_NDK}/sysroot/usr/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android)

    # Add any necessary compile definitions or options for Android
    add_compile_options(-DANDROID -D__ANDROID_API__=${ANDROID_NATIVE_API_LEVEL})

    # Specify the source files for the Android backend
    # NOTE: These files have been commented out and replaced by the PLAIN version in order to develop the Android wrapper code.
    target_include_directories(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android)
    target_sources(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/AdapterBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/PeripheralBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/BluetoothDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/BluetoothGatt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/BluetoothGattService.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/BluetoothGattCharacteristic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/BluetoothGattDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/UUID.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/android/ScanResult.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/bridge/BluetoothGattCallback.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/android/bridge/ScanCallback.cpp
    )

    target_include_directories(simpleble PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/plain)

    target_link_libraries(simpleble PUBLIC
        android
        nativehelper
        log)
endif()

apply_build_options(simpleble
    "${PRIVATE_COMPILE_DEFINITIONS}"
    "${PRIVATE_COMPILE_OPTIONS}"
    "${PRIVATE_LINK_OPTIONS}"
    "${PUBLIC_LINK_OPTIONS}")

apply_build_options(${SIMPLEBLE-C}
    "${PRIVATE_COMPILE_DEFINITIONS}"
    "${PRIVATE_COMPILE_OPTIONS}"
    "${PRIVATE_LINK_OPTIONS}"
    "${PUBLIC_LINK_OPTIONS}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/simpleble.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/simpleble.pc @ONLY)

if(SIMPLEBLE_TEST)
    message(STATUS "Building Tests")
    find_package(GTest REQUIRED)

    add_executable(simpleble_test
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_bytearray.cpp
    )

    set_target_properties(simpleble_test PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        CXX_STANDARD 17
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

    target_link_libraries(simpleble_test PRIVATE simpleble::simpleble GTest::gtest)
endif()

if(MSVC)
    add_custom_command(TARGET ${SIMPLEBLE-C} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../python_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../java_package/brainflow/src/main/resources/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../csharp_package/brainflow/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../matlab_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../julia_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../rust_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../nodejs_package/brainflow/lib/${FILE_NAME}"
    )
endif(MSVC)
if(UNIX AND NOT ANDROID)
    add_custom_command(TARGET ${SIMPLEBLE-C} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../python_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../julia_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../java_package/brainflow/src/main/resources/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../csharp_package/brainflow/brainflow/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../matlab_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../rust_package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../nodejs_package/brainflow/lib/${FILE_NAME}"
    )
endif(UNIX AND NOT ANDROID)

if(MSVC)
    install(
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/$<CONFIG>/${FILE_NAME}
        DESTINATION lib
    )
endif(MSVC)
if(UNIX AND NOT ANDROID)
    install(
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../compiled/${FILE_NAME}
        DESTINATION lib
    )
endif(UNIX AND NOT ANDROID)
