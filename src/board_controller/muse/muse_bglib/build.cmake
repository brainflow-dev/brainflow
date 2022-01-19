if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (MUSE_BLED_LIB "MuseLib")
    if (APPLE)
        SET (MUSE_BLED_LIB_NAME "libMuseLib.dylib")
    elseif (UNIX)
        SET (MUSE_BLED_LIB_NAME "libMuseLib.so")
    else ()
        SET (MUSE_BLED_LIB_NAME "MuseLib.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("32 bits compiler detected")
    SET (PLATFORM_ACH "X86")
    if (APPLE)
        SET (MUSE_BLED_LIB "MuseLib")
        SET (MUSE_BLED_LIB_NAME "libMuseLib.dylib")
    elseif (UNIX)
        SET (MUSE_BLED_LIB "MuseLib")
        SET (MUSE_BLED_LIB_NAME "libMuseLib.so")
    else ()
        SET (MUSE_BLED_LIB "MuseLib32")
        SET (MUSE_BLED_LIB_NAME "MuseLib32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (MUSE_BLED_SOURCE_LIB
    ${CMAKE_CURRENT_LIST_DIR}/callbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd_def.cpp
    ${CMAKE_CURRENT_LIST_DIR}/muse_bglib_helper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stubs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uart.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/timestamp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/data_buffer.cpp
)

add_library (${MUSE_BLED_LIB} SHARED ${MUSE_BLED_SOURCE_LIB})
target_include_directories (${MUSE_BLED_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/utils/inc>
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/muse_bglib/inc>
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/inc>
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/board_controller/inc>
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/third_party/json>
)
set_property (TARGET ${MUSE_BLED_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties (${MUSE_BLED_LIB}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${MUSE_BLED_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${MUSE_BLED_LIB_NAME}"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${MUSE_BLED_LIB} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${MUSE_BLED_LIB_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${MUSE_BLED_LIB_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${MUSE_BLED_LIB_NAME}"
    )
endif (UNIX AND NOT ANDROID)

install (
    TARGETS ${MUSE_BLED_LIB}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)