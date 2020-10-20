cmake_minimum_required (VERSION 3.10)
project (brainflow)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (BRAINFLOW_VERSION 2.1.0)

# dont export sumbols on unix by default
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set (CMAKE_C_VISIBILITY_PRESET hidden)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

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
configure_msvc_runtime ()

# add_subfolder doesnt work on linux if you need to istall packages, add it in top level file
SET (DSPFILTERS "DSPFilters")
aux_source_directory (${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/source DSPFILTERS_SOURCE_LIB)
add_library (${DSPFILTERS} STATIC ${DSPFILTERS_SOURCE_LIB})
target_include_directories (${DSPFILTERS} PUBLIC $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/include>)
set_property (TARGET ${DSPFILTERS} PROPERTY POSITION_INDEPENDENT_CODE ON)
# there are asserts, I wanna disable them even for debug builds
target_compile_definitions(${DSPFILTERS} PRIVATE -DNDEBUG)

SET (WAVELIB "WaveLib")
aux_source_directory (${CMAKE_HOME_DIRECTORY}/third_party/wavelib/src WAVELIB_SRC)
add_library (${WAVELIB} STATIC ${WAVELIB_SRC})
target_include_directories (${WAVELIB} PUBLIC $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/third_party/wavelib/header>)
set_property (TARGET ${WAVELIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (PLATFORM_ACH "X64")
    SET (GANGLION_LIB "GanglionLib")
    SET (BOARD_CONTROLLER_NAME "BoardController")
    SET (DATA_HANDLER_NAME "DataHandler")
    SET (ML_MODULE_NAME "MLModule")
    SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
    if(APPLE)
        SET (GANGLION_LIB_NAME "libGanglionLib.dylib")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.dylib")
        SET (ML_MODULE_COMPILED_NAME "libMLModule.dylib")
        SET (NEUROSDK_LIB_NAME "neurosdk-shared")
    elseif (UNIX)
        SET (GANGLION_LIB_NAME "libGanglionLib.so")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.so")
        SET (ML_MODULE_COMPILED_NAME "libMLModule.so")
    else ()
        SET (GANGLION_LIB_NAME "GanglionLib.dll")
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController.dll")
        SET (DATA_HANDLER_COMPILED_NAME "DataHandler.dll")
        SET (NEUROSDK_LIB_NAME "neurosdk-x64")
        SET (ML_MODULE_COMPILED_NAME "MLModule.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("32 bits compiler detected")
    SET (PLATFORM_ACH "X86")
    # for linux and apple we dont add 32bit library to packages, so there can not be a conflict between libs
    # to dont write code like if(32 bits) if(64 bits) in bindings and keep the ability to use them if brainflow was built from src
    # use the same name for 32 and 64 bits
    if (APPLE)
        SET (GANGLION_LIB "GanglionLib")
        SET (GANGLION_LIB_NAME "libGanglionLib.dylib")
        SET (BOARD_CONTROLLER_NAME "BoardController")
        SET (DATA_HANDLER_NAME "DataHandler")
        SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.dylib")
        SET (ML_MODULE_NAME "MLModule")
        SET (ML_MODULE_COMPILED_NAME "libMLModule.dylib")
    elseif (UNIX)
        SET (GANGLION_LIB "GanglionLib")
        SET (GANGLION_LIB_NAME "libGanglionLib.so")
        SET (BOARD_CONTROLLER_NAME "BoardController")
        SET (DATA_HANDLER_NAME "DataHandler")
        SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.so")
        SET (ML_MODULE_NAME "MLModule")
        SET (ML_MODULE_COMPILED_NAME "libMLModule.so")
    else ()
        SET (GANGLION_LIB "GanglionLib32")
        SET (GANGLION_LIB_NAME "GanglionLib32.dll")
        SET (BOARD_CONTROLLER_NAME "BoardController32")
        SET (DATA_HANDLER_NAME "DataHandler32")
        SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow32")
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController32.dll")
        SET (DATA_HANDLER_COMPILED_NAME "DataHandler32.dll")
        SET (NEUROSDK_LIB_NAME "neurosdk-x86")
        SET (ML_MODULE_NAME "MLModule32")
        SET (ML_MODULE_COMPILED_NAME "MLModule32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

set (GANGLION_SOURCE_LIB
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/callbacks.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/cmd_def.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/helpers.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/main.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/stubs.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/uart.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/timestamp.cpp
)
add_library (${GANGLION_LIB} SHARED ${GANGLION_SOURCE_LIB})
target_include_directories (${GANGLION_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/utils/inc>
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/inc>
)
set_property (TARGET ${GANGLION_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

set (BOARD_CONTROLLER_SRC
    ${CMAKE_HOME_DIRECTORY}/src/utils/timestamp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/data_buffer.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/serial.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_client_tcp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_client_udp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_server_tcp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_server_udp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/multicast_client.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/multicast_server.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/broadcast_client.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/openbci_serial_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/openbci_wifi_shield_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_wifi.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton_wifi.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton_daisy_wifi.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton_daisy.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/board_controller.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/board_info_getter.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/streaming_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/synthetic_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/playback_file_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/novaxr.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/file_streamer.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/multicast_streamer.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/gtec/unicorn_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/neuromd_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/brainbit.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri_eeg.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri_ecg.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri_emg.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/mit/fascia.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neurosity/notion_osc.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/ironbci/ironbci.cpp
)

set (DATA_HANDLER_SRC
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/data_handler.cpp
)

set (ML_MODULE_SRC
    ${CMAKE_HOME_DIRECTORY}/src/ml/ml_module.cpp
    ${CMAKE_HOME_DIRECTORY}/src/ml/concentration_regression_classifier.cpp
    ${CMAKE_HOME_DIRECTORY}/src/ml/base_classifier.cpp
    ${CMAKE_HOME_DIRECTORY}/third_party/libsvm/svm.cpp
    ${CMAKE_HOME_DIRECTORY}/src/ml/concentration_knn_classifier.cpp
    ${CMAKE_HOME_DIRECTORY}/src/ml/concentration_svm_classifier.cpp
)

add_library (
    ${BOARD_CONTROLLER_NAME} SHARED
    ${BOARD_CONTROLLER_SRC}
)

add_library (
    ${DATA_HANDLER_NAME} SHARED
    ${DATA_HANDLER_SRC}
)

add_library (
    ${ML_MODULE_NAME} SHARED
    ${ML_MODULE_SRC}
)

target_include_directories (
    ${BOARD_CONTROLLER_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/third_party/
    ${CMAKE_HOME_DIRECTORY}/third_party/json
    ${CMAKE_HOME_DIRECTORY}/third_party/http
    ${CMAKE_HOME_DIRECTORY}/third_party/unicorn/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/oscpp/include
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/gtec/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/mit/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/inc/types
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neurosity/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/ironbci/inc
)

# disable min max defines in windows.h
target_compile_definitions(${BOARD_CONTROLLER_NAME} PRIVATE -DNOMINMAX)

target_include_directories (
    ${DATA_HANDLER_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/third_party/
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/include
    ${CMAKE_HOME_DIRECTORY}/third_party/wavelib/header
    ${CMAKE_HOME_DIRECTORY}/third_party/fft/src
)

target_include_directories (
    ${ML_MODULE_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/third_party/
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/ml/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/libsvm
    ${CMAKE_HOME_DIRECTORY}/third_party/json
    ${CMAKE_HOME_DIRECTORY}/third_party/kdtree
)

set_target_properties (${BOARD_CONTROLLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

set_target_properties (${GANGLION_LIB}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

set_target_properties (${DATA_HANDLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

set_target_properties (${ML_MODULE_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

# dont link pthread for Android
if (UNIX AND NOT ANDROID)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE pthread dl)
    target_link_libraries (${ML_MODULE_NAME} PRIVATE pthread dl)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE ${DSPFILTERS} ${WAVELIB} pthread dl)
else (UNIX AND NOT ANDROID)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE ${DSPFILTERS} ${WAVELIB})
endif (UNIX AND NOT ANDROID)
# link android logging library
if (ANDROID)
    find_library (log-lib log)
    target_link_libraries (${ML_MODULE_NAME} PRIVATE log)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE log)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE  log)
endif (ANDROID)

add_library (
    ${BRAINFLOW_CPP_BINDING_NAME} STATIC
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/board_shim.cpp
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/ml_model.cpp
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/data_filter.cpp
)

target_include_directories (
    ${BRAINFLOW_CPP_BINDING_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/inc
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/json
    ${CMAKE_HOME_DIRECTORY}/src/ml/inc
)

if (UNIX AND NOT ANDROID)
    target_link_libraries (${BRAINFLOW_CPP_BINDING_NAME} pthread ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME} ${ML_MODULE_NAME})
else (UNIX AND NOT ANDROID)
    target_link_libraries (${BRAINFLOW_CPP_BINDING_NAME} ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME} ${ML_MODULE_NAME})
endif (UNIX AND NOT ANDROID)

# copy
if (MSVC)
    add_custom_command (TARGET ${GANGLION_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${GANGLION_LIB_NAME}"
    )
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        # copy board controller libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_info_getter.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
    )
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        # copy data handler libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/data_handler.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
    )
    add_custom_command (TARGET ${ML_MODULE_NAME} POST_BUILD
        # copy data handler libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/ml/inc/ml_module.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/ml_module.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${GANGLION_LIB_NAME}"
    )
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        # copy board controller libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_info_getter.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_constants.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/brainflow_constants.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
    )
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        # copy data handler libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/data_handler.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
    )
    add_custom_command (TARGET ${ML_MODULE_NAME} POST_BUILD
        # copy ml libs
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/ml/inc/ml_module.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/ml_module.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
    )
endif (UNIX AND NOT ANDROID)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    # copy libunicorn.so
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD 
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/linunicorn.so"
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
if (APPLE)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        # copy neurosdk lib for MacOS
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
    )
endif (APPLE)

if (ANDROID)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/tools/jniLibs/${ANDROID_ABI}/${BOARD_CONTROLLER_COMPILED_NAME}"
    )
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/tools/jniLibs/${ANDROID_ABI}/${DATA_HANDLER_COMPILED_NAME}"
    )
    add_custom_command (TARGET ${ML_MODULE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/tools/jniLibs/${ANDROID_ABI}/${ML_MODULE_COMPILED_NAME}"
    )
endif (ANDROID)

# install
set (VERSION_CONFIG ${CMAKE_HOME_DIRECTORY}/compiled/brainflowConfigVersion.cmake)
set (CONFIG_INSTALL_DIR lib/cmake/brainflow)
set (PROJECT_CONFIG ${CMAKE_HOME_DIRECTORY}/compiled/brainflowConfig.cmake)
set (TARGETS_EXPORT_NAME brainflowTargets)
set (PACKAGE_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set (PACKAGE_INC_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/inc)

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
    FILES
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_constants.h
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_info_getter.h
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/brainflow_input_params.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export.h
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h
    ${CMAKE_HOME_DIRECTORY}/src/ml/inc/ml_module.h
    ${CMAKE_HOME_DIRECTORY}/src/ml/inc/brainflow_model_params.h
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/data_filter.h
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/board_shim.h
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/brainflow_exception.h
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/ml_model.h
    DESTINATION inc
)

# install neurosdk
if (MSVC)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll
        DESTINATION lib
    )
endif (MSVC)
if (APPLE)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib
        DESTINATION lib
    )
endif (APPLE)

# install unicorn lib
if (UNIX AND NOT APPLE)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE)

install (
    TARGETS ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME} ${DSPFILTERS} ${WAVELIB} ${GANGLION_LIB} ${ML_MODULE_NAME} ${BRAINFLOW_CPP_BINDING_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
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