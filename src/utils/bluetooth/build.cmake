if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (BLUETOOTH_LIB_NAME "BrainFlowBluetooth")
    if (APPLE)
        SET (BLUETOOTH_LIB_COMPILED_NAME "libBrainFlowBluetooth.dylib")
    elseif (UNIX)
        SET (BLUETOOTH_LIB_COMPILED_NAME "libBrainFlowBluetooth.so")
    else ()
        SET (BLUETOOTH_LIB_COMPILED_NAME "BrainFlowBluetooth.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (BLUETOOTH_LIB_NAME "BrainFlowBluetooth")
        SET (BLUETOOTH_LIB_COMPILED_NAME "libBrainFlowBluetooth.dylib")
    elseif (UNIX)
        SET (BLUETOOTH_LIB_NAME "BrainFlowBluetooth")
        SET (BLUETOOTH_LIB_COMPILED_NAME "libBrainFlowBluetooth.so")
    else ()
        SET (BLUETOOTH_LIB_NAME "BrainFlowBluetooth32")
        SET (BLUETOOTH_LIB_COMPILED_NAME "BrainFlowBluetooth32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

if (MSVC)
    SET (BLUETOOTH_SRC
        ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/socket_bluetooth_win.cpp
    )
elseif (APPLE)
    SET (BLUETOOTH_SRC
        ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/socket_bluetooth_macos.mm
        ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/macos_third_party/BluetoothDeviceResources.mm
        ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/macos_third_party/BluetoothWorker.mm
        ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/macos_third_party/pipe.c
    )
else ()
    SET (BLUETOOTH_SRC
        ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/socket_bluetooth_linux.cpp
    )
endif (MSVC)

SET (BLUETOOTH_SRC
    ${BLUETOOTH_SRC}
    ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/bluetooth_functions.cpp
)

add_library (
    ${BLUETOOTH_LIB_NAME} SHARED
    ${BLUETOOTH_SRC}
)

target_include_directories (
    ${BLUETOOTH_LIB_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/inc
    ${CMAKE_HOME_DIRECTORY}/src/utils/bluetooth/macos_third_party
)

set_target_properties (${BLUETOOTH_LIB_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${BLUETOOTH_LIB_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${BLUETOOTH_LIB_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${BLUETOOTH_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BLUETOOTH_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BLUETOOTH_LIB_COMPILED_NAME}"
    )
endif (UNIX AND NOT ANDROID)

if (UNIX AND NOT ANDROID AND NOT APPLE)
    target_link_libraries (${BLUETOOTH_LIB_NAME} PRIVATE pthread dl bluetooth)
endif (UNIX AND NOT ANDROID AND NOT APPLE)
if (APPLE)
    target_link_libraries (${BLUETOOTH_LIB_NAME} "-framework foundation -framework IOBluetooth")
endif (APPLE)

install (
    TARGETS ${BLUETOOTH_LIB_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
