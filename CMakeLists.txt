cmake_minimum_required (VERSION 3.13)
project (brainflow)

SET (CMAKE_CXX_STANDARD 11)
SET (CMAKE_VERBOSE_MAKEFILE ON)
SET (BRAINFLOW_VERSION 3.7.3)

SET (CMAKE_CXX_VISIBILITY_PRESET hidden)
SET (CMAKE_C_VISIBILITY_PRESET hidden)
SET (CMAKE_POSITION_INDEPENDENT_CODE ON)
SET (VERSION_CONFIG ${CMAKE_HOME_DIRECTORY}/compiled/brainflowConfigVersion.cmake)
SET (CONFIG_INSTALL_DIR lib/cmake/brainflow)
SET (PROJECT_CONFIG ${CMAKE_HOME_DIRECTORY}/compiled/brainflowConfig.cmake)
SET (TARGETS_EXPORT_NAME brainflowTargets)
SET (PACKAGE_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
SET (PACKAGE_INC_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/inc)

option (USE_LIBFTDI "USE_LIBFTDI" OFF)
option (USE_OPENMP "USE_OPENMP" OFF)
option (WARNINGS_AS_ERRORS "WARNINGS_AS_ERRORS" OFF)
option (BUILD_OYMOTION_SDK "BUILD_OYMOTION_SDK" OFF)
option (BUILD_BLUETOOTH "BUILD_BLUETOOTH" OFF)
option (BUILD_BLE "BUILD_BLE" OFF)
option (USE_PERIPHERY "USE_PERIPHERY" OFF)

include (${CMAKE_HOME_DIRECTORY}/cmake/macros.cmake)
configure_msvc_runtime ()

if (WARNINGS_AS_ERRORS)
    if (MSVC)
        add_compile_options (/WX)
    else ()
        add_compile_options (-Werror -Wno-varargs)
    endif ()
endif (WARNINGS_AS_ERRORS)

include (${CMAKE_HOME_DIRECTORY}/src/board_controller/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/src/data_handler/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/src/ml/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/cpp-package/build.cmake)

include (CMakePackageConfigHelpers)

write_basic_package_version_file (
    ${VERSION_CONFIG}
    VERSION ${BRAINFLOW_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file (
    cmake/Config.cmake.in
    ${PROJECT_CONFIG}
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

install (
    FILES ${PROJECT_CONFIG} ${VERSION_CONFIG}
    DESTINATION ${CONFIG_INSTALL_DIR}
)
install (
    EXPORT ${TARGETS_EXPORT_NAME}
    NAMESPACE brainflow::
    DESTINATION ${CONFIG_INSTALL_DIR}
)
