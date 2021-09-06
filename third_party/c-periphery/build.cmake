SET (PERIPHERY "Periphery")
include (CheckIncludeFiles)
CHECK_INCLUDE_FILES (linux/gpio.h HAVE_CDEV_GPIO_HEADERS)
if (NOT HAVE_CDEV_GPIO_HEADERS)
    message (WARNING "Linux kernel header files not found for character device GPIO support. c-periphery will be built with legacy sysfs GPIO support only.")
endif (NOT HAVE_CDEV_GPIO_HEADERS)

file (GLOB_RECURSE PERIPHERY_SOURCES ${CMAKE_HOME_DIRECTORY}/third_party/c-periphery/src/*.c)

add_library (${PERIPHERY} STATIC ${PERIPHERY_SOURCES})

set_property (TARGET ${PERIPHERY} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions (${PERIPHERY} PRIVATE PERIPHERY_VERSION_COMMIT="BRAINFLOW")
target_compile_definitions (${PERIPHERY} PRIVATE PERIPHERY_GPIO_CDEV_SUPPORT=$<BOOL:${HAVE_CDEV_GPIO_HEADERS}>)
target_include_directories (${PERIPHERY} PRIVATE ${CMAKE_HOME_DIRECTORY}/third-party/c-perphery/src)
include (GNUInstallDirs)

install (
    TARGETS ${PERIPHERY}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
