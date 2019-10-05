cmake_minimum_required (VERSION 3.10)
project (brainflow)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (BRAINFLOW_VERSION 1.0.0)

macro (configure_msvc_runtime)
    if (MSVC)
        # Default to statically-linked runtime.
        if ("${MSVC_RUNTIME}" STREQUAL "")
            set (MSVC_RUNTIME "static")
        endif ()
        # Set compiler options.
        set (variables
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
        )
        if (${MSVC_RUNTIME} STREQUAL "static")
            message(STATUS
                "MSVC -> forcing use of statically-linked runtime."
            )
            foreach (variable ${variables})
                if (${variable} MATCHES "/MD")
                    string (REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
                endif ()
            endforeach ()
        else ()
            message (STATUS
                "MSVC -> forcing use of dynamically-linked runtime."
            )
            foreach (variable ${variables})
                if (${variable} MATCHES "/MT")
                    string (REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
                endif ()
            endforeach ()
        endif ()
    endif ()
endmacro ()

# link msvc runtime statically
if (MSVC)
    configure_msvc_runtime()
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (PLATFORM_ACH "X64")
    SET (BOARD_CONTROLLER_NAME "BoardController")
    SET (GET_DATA_NAME "GetData")
    if(APPLE)
        SET (COMPILED_FILE_NAME "libBoardController.dylib")
    elseif (UNIX)
        SET (COMPILED_FILE_NAME "libBoardController.so")
    else ()
        SET (COMPILED_FILE_NAME "BoardController.dll")
        SET (GANGLION_LIB_NATIVE_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/x64/Release/GanglionLibNative64.dll")
        SET (GANGLION_LIB_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/x64/Release/GanglionLib.dll")
        SET (GANGLION_LIB_NATIVE_NAME "GanglionLibNative64.dll")
        SET (GANGLION_LIB_NAME "GanglionLib.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("32 bits compiler detected")
    SET (PLATFORM_ACH "X86")
    SET (BOARD_CONTROLLER_NAME "BoardController32")
    SET (GET_DATA_NAME "GetData32")
    if (APPLE)
        SET (COMPILED_FILE_NAME "libBoardController32.dylib")
    elseif (UNIX)
        SET (COMPILED_FILE_NAME "libBoardController32.so")
    else ()
        SET (COMPILED_FILE_NAME "BoardController32.dll")
        SET (GANGLION_LIB_NATIVE_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/Win32/Release/GanglionLibNative32.dll")
        SET (GANGLION_LIB_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/Win32/Release/GanglionLib.dll")
        SET (GANGLION_LIB_NATIVE_NAME "GanglionLibNative32.dll")
        SET (GANGLION_LIB_NAME "GanglionLib.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

# boardcontroller
set (LIBRARY_SRC
    "src/utils/timestamp.cpp"
    "src/utils/data_buffer.cpp"
    "src/utils/serial.cpp"
    "src/utils/socket_client.cpp"
    "src/utils/socket_server.cpp"
    "src/board_controller/openbci/openbci_serial_board.cpp"
    "src/board_controller/openbci/openbci_wifi_shield_board.cpp"
    "src/board_controller/openbci/ganglion_wifi.cpp"
    "src/board_controller/openbci/cyton_wifi.cpp"
    "src/board_controller/openbci/cyton_daisy_wifi.cpp"
    "src/board_controller/openbci/ganglion.cpp"
    "src/board_controller/openbci/cyton.cpp"
    "src/board_controller/openbci/cyton_daisy.cpp"
    "src/board_controller/board_controller.cpp"
    "src/board_controller/board.cpp"
    "src/board_controller/synthetic_board.cpp"
    "src/board_controller/openbci/novaxr.cpp"
)

add_library (
    ${BOARD_CONTROLLER_NAME} SHARED
    ${LIBRARY_SRC}
)

target_include_directories (
    ${BOARD_CONTROLLER_NAME} PRIVATE
    inc
    src/utils/
    src/board_controller/inc
    src/board_controller/openbci/inc
    GanglionBLEAPI/Interface
)

set_target_properties (${BOARD_CONTROLLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
)

if (UNIX)
    target_link_libraries (${BOARD_CONTROLLER_NAME} -lpthread -ldl)
endif (UNIX)

# getdata
# it's impossible to install target from subfolder, so we have to move GetData target to top level or split installation
# I prefer to move GetData from subfolder to top level CMakeLists.txt
add_library (
    ${GET_DATA_NAME} STATIC
    cpp-package/src/board_shim.cpp
    cpp-package/src/data_handler.cpp
)

target_include_directories (
    ${GET_DATA_NAME} PRIVATE
    src/board_controller/inc
)

if (UNIX)
    target_link_libraries (${GET_DATA_NAME} pthread ${BOARD_CONTROLLER_NAME})
else (UNIX)
    target_link_libraries (${GET_DATA_NAME} ${BOARD_CONTROLLER_NAME})
endif (UNIX)

# copy
if (MSVC)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD 
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/lib/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${COMPILED_FILE_NAME}"
        # copy libraries which are required for ganglion
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/matlab-package/lib/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/matlab-package/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NAME}"
    )
endif (MSVC)
if (UNIX)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/lib/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${COMPILED_FILE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${COMPILED_FILE_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${COMPILED_FILE_NAME}"
    )
endif (UNIX)

# install
set (version_config "${CMAKE_HOME_DIRECTORY}/compiled/brainflowConfigVersion.cmake")
set (config_install_dir "lib/cmake/brainflow")
set (project_config "${CMAKE_HOME_DIRECTORY}/compiled/brainflowConfig.cmake")
set (targets_export_name brainflowTargets)
set (package_lib_install_dir ${CMAKE_INSTALL_PREFIX}/lib)
set (package_inc_install_dir ${CMAKE_INSTALL_PREFIX}/inc)

include (CMakePackageConfigHelpers)

write_basic_package_version_file (
    "${version_config}"
    VERSION ${BRAINFLOW_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file (
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

install (
    FILES
    "src/board_controller/inc/board_controller.h"
    "cpp-package/src/data_handler.h"
    "cpp-package/src/board_shim.h"
    "cpp-package/src/board_info_getter.h"
    "cpp-package/src/brainflow_exception.h"
    DESTINATION inc
)

install (
    TARGETS ${BOARD_CONTROLLER_NAME} ${GET_DATA_NAME}
    EXPORT ${targets_export_name}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)

if (MSVC)
    install (
        FILES
        "${GANGLION_LIB_NATIVE_PATH}"
        "${GANGLION_LIB_PATH}"
        DESTINATION lib
    )
endif (MSVC)

install (
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install (
    EXPORT ${targets_export_name}
    NAMESPACE brainflow::
    DESTINATION "${config_install_dir}"
)