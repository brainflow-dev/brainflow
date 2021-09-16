# Basic CMake configuration
cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SimpleBLE VERSION 0.1 LANGUAGES CXX)

if(APPLE)
    SET(FILE_NAME "libsimpleble-c.dylib")
elseif(UNIX)
    SET(FILE_NAME "libsimpleble-c.so")
else()
    SET(FILE_NAME "simpleble-c.dll")
endif(APPLE)

# Include all necessary CMake modules
include(FetchContent)

# Detect if the project is being build within a project or standalone.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(STANDALONE true)
    # Configure the build path
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    set(STANDALONE false)
endif()

# Load default parameters passed in through the command line.
if(NOT SIMPLEDBUS_GIT_REPOSITORY)
    set(SIMPLEDBUS_GIT_REPOSITORY "https://github.com/kdewald/SimpleDBus.git")
endif()
if(NOT SIMPLEDBUS_GIT_TAG)
    set(SIMPLEDBUS_GIT_TAG "feature/simpleble")
endif()

# Perform some overall CMake configurations
#set(CMAKE_CXX_STANDARD 17) // TODO: Specify the standard only for this library

# Configure the external header path
set(SIMPLEBLE_USER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Configure common source files
set(
    SRC_SIMPLEBLE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Peripheral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AdapterBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PeripheralBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AdapterSafe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PeripheralSafe.cpp
)

set(
    SRC_SIMPLEBLE_C_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/adapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src_c/peripheral.cpp
)

# Detect the operating system and load the necessary dependencies
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    message("-- [INFO] Linux Host Detected")

    FetchContent_Declare(
        simpledbus
        GIT_REPOSITORY ${SIMPLEDBUS_GIT_REPOSITORY}
        GIT_TAG ${SIMPLEDBUS_GIT_TAG}
    )
    FetchContent_MakeAvailable(simpledbus)

    set(
        EXTERNAL_LIBRARIES
        bluezdbus-static
        pthread
    )
    set(
        EXTERNAL_INCLUDE_DIRS
        ${SIMPLEDBUS_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/linux
    )
    set(
        SRC_SIMPLEBLE_FILES
        ${SRC_SIMPLEBLE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/AdapterBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/PeripheralBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/Bluez.cpp
    )
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    message("-- [INFO] Windows Host Detected")
    if(CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
        set(WINDOWS_TARGET_ARCH x86)
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
        set(WINDOWS_TARGET_ARCH x64)
    endif()

    set(WINVERSION_CODE 0x0A00) # Selected Windows 10 based on https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
    
    # Add all the special definitions that need to be added for the program to properly compile on windows.
    set(SPECIAL_DEFINITIONS "/D_WIN32_WINNT=${WINVERSION_CODE} /D_USE_MATH_DEFINES")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /wd4251 ${SPECIAL_DEFINITIONS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W2 /wd4251 ${SPECIAL_DEFINITIONS}")

    set(
        EXTERNAL_INCLUDE_DIRS
        ${SIMPLEDBUS_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/windows
    )
    set(
        SRC_SIMPLEBLE_FILES
        ${SRC_SIMPLEBLE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/AdapterBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/PeripheralBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/Utils.cpp
    )

elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    message("-- [INFO] Darwin Host Detected")
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(
        EXTERNAL_LIBRARIES
        "-framework Foundation"
        "-framework CoreBluetooth"
        ObjC
    )
    set(
        EXTERNAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/macos
    )
    set(
        SRC_SIMPLEBLE_FILES
        ${SRC_SIMPLEBLE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/Utils.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/AdapterBase.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/AdapterBaseMacOS.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/PeripheralBase.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/macos/PeripheralBaseMacOS.mm
    )
else()
    message(FATAL_ERROR "-- [ERROR] UNSUPPORTED SYSTEM: ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_SYSTEM_NAME}")
endif()

message("-- [INFO] Building SimpleBLE")

set(
    SIMPLE_BLE_ALL_INCLUDE_DIRS
    ${SIMPLE_BLE_ALL_INCLUDE_DIRS}
    ${SIMPLEBLE_USER_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${EXTERNAL_INCLUDE_DIRS}
)

add_library(simpleble-static STATIC ${SRC_SIMPLEBLE_FILES})
add_library(simpleble SHARED ${SRC_SIMPLEBLE_FILES})
add_library(simpleble-c-static STATIC ${SRC_SIMPLEBLE_C_FILES})
add_library(simpleble-c SHARED ${SRC_SIMPLEBLE_C_FILES})

set_property(TARGET simpleble-static PROPERTY CXX_STANDARD 17)
set_property(TARGET simpleble PROPERTY CXX_STANDARD 17)
set_property(TARGET simpleble-c-static PROPERTY CXX_STANDARD 17)
set_property(TARGET simpleble-c PROPERTY CXX_STANDARD 17)
set_property(TARGET simpleble-static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET simpleble PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET simpleble-c-static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET simpleble-c PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(simpleble-static PRIVATE ${SIMPLE_BLE_ALL_INCLUDE_DIRS})
target_include_directories(simpleble PRIVATE ${SIMPLE_BLE_ALL_INCLUDE_DIRS})

target_link_libraries(simpleble-static PUBLIC ${EXTERNAL_LIBRARIES})
target_link_libraries(simpleble PUBLIC ${EXTERNAL_LIBRARIES})

target_include_directories(simpleble-static PRIVATE ${SIMPLE_BLE_ALL_INCLUDE_DIRS})
target_include_directories(simpleble PRIVATE ${SIMPLE_BLE_ALL_INCLUDE_DIRS})
target_include_directories(simpleble-c-static PRIVATE ${SIMPLE_BLE_ALL_INCLUDE_DIRS})
target_include_directories(simpleble-c PRIVATE ${SIMPLE_BLE_ALL_INCLUDE_DIRS})

target_link_libraries(simpleble-static PUBLIC ${EXTERNAL_LIBRARIES})
target_link_libraries(simpleble PUBLIC ${EXTERNAL_LIBRARIES})
target_link_libraries(simpleble-c-static PUBLIC simpleble-static)
target_link_libraries(simpleble-c PUBLIC simpleble-static)

# Export the variables needed by the parent project
if(NOT ${STANDALONE})
    set(SIMPLEBLE_INCLUDES ${SIMPLEBLE_USER_INCLUDE_DIR} PARENT_SCOPE)
endif()
set_target_properties(simpleble-c
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (MSVC)
    add_custom_command (TARGET simpleble-c POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${FILE_NAME}"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET simpleble-c POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${FILE_NAME}"
    )
endif (UNIX AND NOT ANDROID)

if (MSVC)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${FILE_NAME}
        DESTINATION lib
    )
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/compiled/${FILE_NAME}
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
