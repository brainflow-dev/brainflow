if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (BOARD_CONTROLLER_NAME "BoardController")
    if (APPLE)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
    elseif (UNIX)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
    else ()
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController.dll")
        SET (BOARD_CONTROLLER_COMPILED_NAME_DOT_LIB "BoardController.lib")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (BOARD_CONTROLLER_NAME "BoardController")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
    elseif (UNIX)
        SET (BOARD_CONTROLLER_NAME "BoardController")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
    else ()
        SET (BOARD_CONTROLLER_NAME "BoardController32")
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController32.dll")
        SET (BOARD_CONTROLLER_COMPILED_NAME_DOT_LIB "BoardController32.lib")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (BOARD_CONTROLLER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/timestamp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/data_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/os_serial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/os_serial_ioctl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/serial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/libftdi_serial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/socket_client_tcp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/socket_client_udp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/socket_server_tcp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/socket_server_udp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/multicast_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/multicast_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/broadcast_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/broadcast_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/galea_v4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/galea_serial_v4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/galea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/galea_serial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/openbci_serial_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/openbci_wifi_shield_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/ganglion_wifi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/cyton_wifi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/cyton_daisy_wifi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/ganglion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/ganglion_native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/cyton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/cyton_daisy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/board_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/board_info_getter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/brainflow_boards.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/streaming_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/synthetic_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/dyn_lib_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/bt_lib_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/playback_file_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/file_streamer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/multicast_streamer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/plotjuggler_udp_streamer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/gtec/unicorn_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/neuromd_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/brainbit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/callibri.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/callibri_eeg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/callibri_ecg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/callibri_emg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neurosity/notion_osc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/mentalab/explore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/oymotion/gforce_pro.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/oymotion/gforce_dual.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/freeeeg/freeeeg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/brainbit_bled.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/muse/muse_bled.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ant_neuro/ant_neuro.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/enophone/enophone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ble_lib_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/muse/muse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/brainalive/brainalive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/emotibit/emotibit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ntl/ntl_wifi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/aavaa/aavaa_v3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/pieeg/pieeg_board.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuropawn/knight.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/biolistener/biolistener.cpp
)

include (${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ant_neuro/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/ganglion_bglib/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/gtec/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/muse/muse_bglib/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/build.cmake)

if (BUILD_OYMOTION_SDK)
    include (${CMAKE_CURRENT_SOURCE_DIR}/third_party/gForceSDKCXX/build.cmake)
endif (BUILD_OYMOTION_SDK)

if (BUILD_BLUETOOTH)
    include (${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/build.cmake)
endif (BUILD_BLUETOOTH)

if (BUILD_BLE)
    add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/third_party/SimpleBLE/simpleble)
endif (BUILD_BLE)

add_library (
    ${BOARD_CONTROLLER_NAME} SHARED
    ${BOARD_CONTROLLER_SRC}
)

target_include_directories (
    ${BOARD_CONTROLLER_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/http
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/oscpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/bluetooth/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/oymotion/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/gtec/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/openbci/ganglion_bglib/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuromd/brainbit_bglib/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/muse/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/muse/muse_bglib/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/neurosdk/inc/types
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neurosity/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gForceSDKCXX/src/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/freeeeg/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ant_neuro/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/enophone/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/SimpleBLE/simpleble/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/brainalive/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/mentalab/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/emotibit/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/ntl/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/aavaa/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/pieeg/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/neuropawn/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/biolistener/inc
)

target_compile_definitions(${BOARD_CONTROLLER_NAME} PRIVATE NOMINMAX BRAINFLOW_VERSION=${BRAINFLOW_VERSION})

set_target_properties (${BOARD_CONTROLLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
)

if (USE_LIBFTDI)
    find_package (LibFTDI1 NO_MODULE)
    if (LibFTDI1_FOUND)
        # this line could SET up the directories if included before targets are defined
        #include ( ${LIBFTDI_USE_FILE} )
        # this works around a bug in some installations of libftdi++ 1.5
        # http://developer.intra2net.com/git/?p=libftdi;a=commit;h=cdb28383402d248dbc6062f4391b038375c52385
        SET (LIBFTDI_INCLUDE_DIRS ${LIBFTDI_INCLUDE_DIRS} ${LIBFTDI_INCLUDE_DIR}/../libftdi1)
        target_compile_definitions (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_DEFINITIONS} "USE_LIBFTDI")
        target_include_directories (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_INCLUDE_DIRS})
        target_link_directories (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_LIBRARY_DIRS})
        target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_LIBRARIES})
    else (LibFTDI1_FOUND)
        message (FATAL_ERROR "USE_LIBFTDI SET but LibFTDI not found.")
    endif (LibFTDI1_FOUND)
endif (USE_LIBFTDI)

if (BUILD_PERIPHERY)
    add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/third_party/periphery)
    target_compile_definitions (${BOARD_CONTROLLER_NAME} PRIVATE USE_PERIPHERY)
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/third_party/periphery)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE periphery ${CMAKE_THREAD_LIBS_INIT} dl)
endif (BUILD_PERIPHERY)

if (MSVC)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_controller.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_info_getter.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME_DOT_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME_DOT_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_controller.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_info_getter.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/brainflow_constants.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/brainflow_constants.h"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_controller.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_info_getter.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/brainflow_constants.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/brainflow_constants.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_controller.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_info_getter.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/brainflow_constants.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/brainflow_constants.h"
    )
endif (UNIX AND NOT ANDROID)

if (ANDROID)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/jniLibs/${ANDROID_ABI}/${BOARD_CONTROLLER_COMPILED_NAME}"
    )
    if (LibFTDI1_FOUND)
        add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/java_package/android/src/main/libs/${ANDROID_ABI}/"
        )
        foreach (DEPENDENCY_LIBRARY ${LIBFTDI_LIBRARIES})
            string (REGEX REPLACE "^-l" "" DEPENDENCY_LIBRARY ${DEPENDENCY_LIBRARY})
            if (DEPENDENCY_LIBRARY STREQUAL "log")
                continue ()
            endif (DEPENDENCY_LIBRARY STREQUAL "log")
            find_library (DEPENDENCY_LIBRARY_LOCATION NAMES ${DEPENDENCY_LIBRARY} HINTS ${LIBFTDI_LIBRARY_DIRS} / NO_CMAKE_PATH NO_CMAKE_FIND_ROOT_PATH)
            add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEPENDENCY_LIBRARY_LOCATION}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/jniLibs/${ANDROID_ABI}/"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEPENDENCY_LIBRARY_LOCATION}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/android/src/main/libs/${ANDROID_ABI}/"
            )
            unset (DEPENDENCY_LIBRARY_LOCATION CACHE)
        endforeach (DEPENDENCY_LIBRARY)
    endif (LibFTDI1_FOUND)
endif (ANDROID)

if (UNIX AND NOT ANDROID)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE pthread dl)
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    find_library (log-lib log)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE log)
endif (ANDROID)

install (
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/brainflow_constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/board_info_getter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc/brainflow_input_params.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/brainflow_array.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/brainflow_exception.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/json.hpp
    DESTINATION inc
)

install (
    TARGETS ${BOARD_CONTROLLER_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
