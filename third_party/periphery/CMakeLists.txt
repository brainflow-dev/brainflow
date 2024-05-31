cmake_minimum_required(VERSION 2.6)
project(periphery C)

option(BUILD_TESTS "Build test programs" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Check Linux kernel header files for character device GPIO support
include(CheckSourceCompiles)
check_source_compiles(C "#include <linux/gpio.h>\nint main(void) { GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME; return 0; }" HAVE_GPIO_CDEV_V2)
include(CheckSymbolExists)
check_symbol_exists(GPIO_GET_LINEEVENT_IOCTL linux/gpio.h HAVE_GPIO_CDEV_V1)
if(HAVE_GPIO_CDEV_V2)
    set(GPIO_CDEV_SUPPORT 2)
elseif(HAVE_GPIO_CDEV_V1)
    set(GPIO_CDEV_SUPPORT 1)
else()
    set(GPIO_CDEV_SUPPORT 0)
    message(WARNING "Missing character device GPIO support in Linux kernel header files. c-periphery will be built with legacy sysfs GPIO support only.")
endif()
add_definitions(-DPERIPHERY_GPIO_CDEV_SUPPORT=${GPIO_CDEV_SUPPORT})

# Library version
set(VERSION "2.4.2")
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
# Conditional flags for different compilers
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /WX")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic -Wall -Wextra -Wno-stringop-truncation -fPIC")
endif()
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
