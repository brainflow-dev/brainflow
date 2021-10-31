cmake_minimum_required(VERSION 2.6)
project(periphery C)

option(BUILD_TESTS "Build test programs" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Check for Linux kernel header files for character device GPIO support
include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(linux/gpio.h HAVE_CDEV_GPIO_HEADERS)
if(NOT HAVE_CDEV_GPIO_HEADERS)
message(WARNING "Linux kernel header files not found for character device GPIO support. c-periphery will be built with legacy sysfs GPIO support only.")
endif()
add_definitions(-DPERIPHERY_GPIO_CDEV_SUPPORT=$<BOOL:${HAVE_CDEV_GPIO_HEADERS}>)

# Library version
set(VERSION "2.3.1")
set(SOVERSION ${VERSION})

# Glob sources, headers, tests
file(GLOB_RECURSE periphery_SOURCES src/*.c)
file(GLOB_RECURSE periphery_HEADERS src/*.h)
file(GLOB_RECURSE periphery_TESTS tests/*.c)

# Expose git commit id into COMMIT_ID variable
execute_process(
    COMMAND git --git-dir="${CMAKE_CURRENT_SOURCE_DIR}/.git" describe --abbrev --always --tags --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE COMMIT_ID
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Define C flags and include directories
add_definitions(-DPERIPHERY_VERSION_COMMIT="${COMMIT_ID}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic -Wall -Wextra -Wno-stringop-truncation -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Declare library target
add_library(periphery ${periphery_SOURCES} ${periphery_HEADERS})
set_target_properties(periphery PROPERTIES SOVERSION ${VERSION})

include(GNUInstallDirs)

# Generate pkg-config pc file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/libperiphery.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libperiphery.pc @ONLY)

# Declare install targets
install(TARGETS periphery DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${periphery_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libperiphery.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Declare test targets if enabled
if(BUILD_TESTS)
    foreach(TEST_SOURCE ${periphery_TESTS})
        get_filename_component(TEST_PROGRAM ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_PROGRAM} ${TEST_SOURCE})
        target_link_libraries(${TEST_PROGRAM} periphery pthread)
        set(TEST_PROGRAMS ${TEST_PROGRAMS} ${TEST_PROGRAM})
    endforeach()
    add_custom_target(tests DEPENDS periphery ${TEST_PROGRAMS})
endif()
