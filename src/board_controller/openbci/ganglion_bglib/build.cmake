if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (GANGLION_LIB "GanglionLib")
    if (APPLE)
        SET (GANGLION_LIB_NAME "libGanglionLib.dylib")
    elseif (UNIX)
        SET (GANGLION_LIB_NAME "libGanglionLib.so") 
    else ()
        SET (GANGLION_LIB_NAME "GanglionLib.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (GANGLION_LIB "GanglionLib")
        SET (GANGLION_LIB_NAME "libGanglionLib.dylib")
    elseif (UNIX)
        SET (GANGLION_LIB "GanglionLib")
        SET (GANGLION_LIB_NAME "libGanglionLib.so")
    else ()
        SET (GANGLION_LIB "GanglionLib32")
        SET (GANGLION_LIB_NAME "GanglionLib32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (GANGLION_SOURCE_LIB
    ${CMAKE_CURRENT_LIST_DIR}/callbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd_def.cpp
    ${CMAKE_CURRENT_LIST_DIR}/helpers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stubs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/timestamp.cpp
)

add_library (${GANGLION_LIB} SHARED ${GANGLION_SOURCE_LIB})
target_include_directories (${GANGLION_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)
set_property (TARGET ${GANGLION_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties (${GANGLION_LIB}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${GANGLION_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${GANGLION_LIB_NAME}"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${GANGLION_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/${GANGLION_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${GANGLION_LIB_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${GANGLION_LIB_NAME}"
    )
endif (UNIX AND NOT ANDROID)

install (
    TARGETS ${GANGLION_LIB}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
