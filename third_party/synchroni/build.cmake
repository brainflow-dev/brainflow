SET (SYNCHRONI_SDK_WRAPPER_NAME "SynchroniLib")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.dylib")
    elseif (UNIX)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.so")
    else ()
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "SynchroniLib.dll")
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME_DOT_LIB "SynchroniLib.lib")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.dylib")
    elseif (UNIX)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.so")
    else ()
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "SynchroniLib32.dll")
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME_DOT_LIB "SynchroniLib32.lib")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)


SET (SYNCHRONI_WRAPPER_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/synchroni_wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/timestamp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/brainflow_boards.cpp
)

add_library (
    ${SYNCHRONI_SDK_WRAPPER_NAME} SHARED
    ${SYNCHRONI_WRAPPER_SRC}
)

if (APPLE)
    target_link_libraries (${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/mac/sensor.xcframework)
endif (APPLE)

target_compile_definitions(${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE -DUNICODE -D_UNICODE)

target_include_directories (
    ${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json
)

#find_library (SYNCHRONI_SDK NAMES ${SYNCHRONI_SDK_NAME} PATHS ${CMAKE_CURRENT_LIST_DIR}/lib)
#target_link_libraries (${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE ${SYNCHRONI_SDK})


set_target_properties (${SYNCHRONI_SDK_WRAPPER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${SYNCHRONI_SDK_WRAPPER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/src/main/resources/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${SYNCHRONI_SDK_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_NAME}.dll"
    )
endif (MSVC)

if (UNIX)
    add_custom_command (TARGET ${SYNCHRONI_SDK_WRAPPER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
    )
endif (UNIX)

if (MSVC)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        install (
            FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/gForceSDKWrapper.dll
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gForceSDKCXX/lib/gforce64.dll
            DESTINATION lib
        )
    else (CMAKE_SIZEOF_VOID_P EQUAL 8)
        install (
            FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/gForceSDKWrapper32.dll
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gForceSDKCXX/lib/gforce32.dll
            DESTINATION lib
        )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (MSVC)

install (
    TARGETS ${SYNCHRONI_SDK_WRAPPER_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)