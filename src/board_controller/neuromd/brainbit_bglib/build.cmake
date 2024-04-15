if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (BRAINBIT_BLED_LIB "BrainBitLib")
    if (APPLE)
        SET (BRAINBIT_BLED_LIB_NAME "libBrainBitLib.dylib")
    elseif (UNIX)
        SET (BRAINBIT_BLED_LIB_NAME "libBrainBitLib.so")
    else ()
        SET (BRAINBIT_BLED_LIB_NAME "BrainBitLib.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (BRAINBIT_BLED_LIB "BrainBitLib")
        SET (BRAINBIT_BLED_LIB_NAME "libBrainBitLib.dylib")
    elseif (UNIX)
        SET (BRAINBIT_BLED_LIB "BrainBitLib")
        SET (BRAINBIT_BLED_LIB_NAME "libBrainBitLib.so")
    else ()
        SET (BRAINBIT_BLED_LIB "BrainBitLib32")
        SET (BRAINBIT_BLED_LIB_NAME "BrainBitLib32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (BRAINBIT_BLED_SOURCE_LIB
    ${CMAKE_CURRENT_LIST_DIR}/callbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd_def.cpp
    ${CMAKE_CURRENT_LIST_DIR}/helpers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stubs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/timestamp.cpp
)

add_library (${BRAINBIT_BLED_LIB} SHARED ${BRAINBIT_BLED_SOURCE_LIB})
target_include_directories (${BRAINBIT_BLED_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/json>
)
set_property (TARGET ${BRAINBIT_BLED_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties (${BRAINBIT_BLED_LIB}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${BRAINBIT_BLED_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${BRAINBIT_BLED_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/${BRAINBIT_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${BRAINBIT_BLED_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${BRAINBIT_BLED_LIB_NAME}"
    )
endif (UNIX AND NOT ANDROID)

install (
    TARGETS ${BRAINBIT_BLED_LIB}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)