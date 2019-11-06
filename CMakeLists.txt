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

# add_subfolder doesnt work if you need to istall packages, add it in top level file
SET (DSPFilters "DSPFilters")
aux_source_directory (${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/source DSPFILTERS_SOURCE_LIB)
add_library (${DSPFilters} STATIC ${DSPFILTERS_SOURCE_LIB})
target_include_directories (${DSPFilters} PUBLIC $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/include>)
set_property (TARGET ${DSPFilters} PROPERTY POSITION_INDEPENDENT_CODE ON)

SET (BOARDS_JSON "brainflow_boards.json")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (PLATFORM_ACH "X64")
    SET (BOARD_CONTROLLER_NAME "BoardController")
    SET (DATA_HANDLER_NAME "DataHandler")
    SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
    if(APPLE)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.dylib")
    elseif (UNIX)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
        SET (GANGLION_LIB_NAME "GanglionLibrary.so")
        SET (GANGLION_LIB_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/GanglionLibrary.so")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.so")
    else ()
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController.dll")
        SET (DATA_HANDLER_COMPILED_NAME "DataHandler.dll")
        SET (GANGLION_LIB_NATIVE_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/x64/Release/GanglionLibNative64.dll")
        SET (GANGLION_LIB_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/x64/Release/GanglionLib.dll")
        SET (GANGLION_LIB_NATIVE_NAME "GanglionLibNative64.dll")
        SET (GANGLION_LIB_NAME "GanglionLib.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("32 bits compiler detected")
    SET (PLATFORM_ACH "X86")
    SET (BOARD_CONTROLLER_NAME "BoardController32")
    SET (DATA_HANDLER_NAME "DataHandler32")
    SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow32")
    if (APPLE)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController32.dylib")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler32.dylib")
    elseif (UNIX)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController32.so")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler32.so")
    else ()
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController32.dll")
        SET (DATA_HANDLER_COMPILED_NAME "DataHandler32.dll")
        SET (GANGLION_LIB_NATIVE_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/Win32/Release/GanglionLibNative32.dll")
        SET (GANGLION_LIB_PATH "${CMAKE_HOME_DIRECTORY}/GanglionBLEAPI/Win32/Release/GanglionLib.dll")
        SET (GANGLION_LIB_NATIVE_NAME "GanglionLibNative32.dll")
        SET (GANGLION_LIB_NAME "GanglionLib.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

set (BOARD_CONTROLLER_SRC
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
    "src/board_controller/board_info_getter.cpp"
    "src/board_controller/board.cpp"
    "src/board_controller/synthetic_board.cpp"
    "src/board_controller/openbci/novaxr.cpp"
)

set (DATA_HANDLER_SRC
    "src/data_handler/data_handler.cpp"
)

add_library (
    ${BOARD_CONTROLLER_NAME} SHARED
    ${BOARD_CONTROLLER_SRC}
)

add_library (
    ${DATA_HANDLER_NAME} SHARED
    ${DATA_HANDLER_SRC}
)

target_include_directories (
    ${BOARD_CONTROLLER_NAME} PRIVATE
    third_party/
    third_party/json
    third_party/http
    src/utils/
    src/board_controller/inc
    src/board_controller/openbci/inc
    GanglionBLEAPI/Interface
)

target_include_directories (
    ${DATA_HANDLER_NAME} PRIVATE
    third_party/
    src/utils/
    src/board_controller/inc
    src/data_handler/inc
    third_party/DSPFilters/include
)

set_target_properties (${BOARD_CONTROLLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
)

set_target_properties (${DATA_HANDLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
)

if (UNIX)
    target_link_libraries (${BOARD_CONTROLLER_NAME} pthread dl)
    target_link_libraries (${DATA_HANDLER_NAME} ${DSPFilters} pthread dl)
else (UNIX)
    target_link_libraries (${DATA_HANDLER_NAME} ${DSPFilters})
endif (UNIX)

# getdata
# it's impossible to install target from subfolder, so we have to move GetData target to top level or split installation
# I prefer to move GetData from subfolder to top level CMakeLists.txt
add_library (
    ${BRAINFLOW_CPP_BINDING_NAME} STATIC
    cpp-package/src/board_shim.cpp
    cpp-package/src/data_filter.cpp
)

target_include_directories (
    ${BRAINFLOW_CPP_BINDING_NAME} PRIVATE
    src/board_controller/inc
    src/data_handler/inc
    third_party/json
)

if (UNIX)
    target_link_libraries (${BRAINFLOW_CPP_BINDING_NAME} pthread ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME})
else (UNIX)
    target_link_libraries (${BRAINFLOW_CPP_BINDING_NAME} ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME})
endif (UNIX)

# copy
if (MSVC)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD 
        # copy board controller libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        # copy libraries which are required for ganglion
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_NATIVE_PATH}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NATIVE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NAME}"    
    )
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD 
        # copy data handler libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
    )
endif (MSVC)
if (UNIX)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        # copy board controller libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
    )
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        # copy data handler libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
    )
endif (UNIX)
if (UNIX AND NOT APPLE)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        # copy libraries for ganglion
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GANGLION_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NAME}"
    )
endif (UNIX AND NOT APPLE)

# need to copy json file to bindings
add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/${BOARDS_JSON}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARDS_JSON}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/${BOARDS_JSON}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARDS_JSON}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/${BOARDS_JSON}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${BOARDS_JSON}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/${BOARDS_JSON}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARDS_JSON}"
)

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
    "src/board_controller/inc/brainflow_constants.h"
    "src/board_controller/inc/board_info_getter.h"
    "src/data_handler/inc/data_handler.h"
    "cpp-package/src/data_filter.h"
    "cpp-package/src/board_shim.h"
    "cpp-package/src/brainflow_exception.h"
    DESTINATION inc
)

# need to put this file with libraries
install (
    FILES
    ${BOARDS_JSON}
    DESTINATION lib
)

install (
    TARGETS ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME} ${DSPFilters} ${BRAINFLOW_CPP_BINDING_NAME}
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
if (UNIX AND NOT APPLE)
    install (
        FILES
        "${GANGLION_LIB_PATH}"
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE)

install (
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install (
    EXPORT ${targets_export_name}
    NAMESPACE brainflow::
    DESTINATION "${config_install_dir}"
)