SET (SYNCHRONI_SDK_WRAPPER_NAME "SynchroniLib")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.dylib")
    elseif (UNIX)
        if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
            set(SYNCHRONI_SDK_WRAPPER_NAME "SynchroniLib_arm64") 
            SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib_arm64.so")
            SET (SYNCHRONI_SDK_ARCH "arm64")
            SET (SYNCHRONI_SDK_LINK_NAME_LIB "libsensor_arm64.so")
        else ()
            set(SYNCHRONI_SDK_WRAPPER_NAME "SynchroniLib_x64") 
            SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib_x64.so")
            SET (SYNCHRONI_SDK_ARCH "x86_64")
            SET (SYNCHRONI_SDK_LINK_NAME_LIB "libsensor_x64.so")
        endif ()
    else ()
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "SynchroniLib.dll")
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME_DOT_LIB "SynchroniLib.lib")
        SET (SYNCHRONI_SDK_LINK_NAME_LIB "sensor")
        SET (SYNCHRONI_SDK_LINK_NAME_DOT_LIB "sensor.lib")
        SET (SYNCHRONI_SDK_ARCH "Win64")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.dylib")
    elseif (UNIX)
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "libSynchroniLib.so")
    else ()
        SET (SYNCHRONI_SDK_WRAPPER_NAME "SynchroniLib32")
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME "SynchroniLib32.dll")
        SET (SYNCHRONI_SDK_WRAPPER_COMPILED_NAME_DOT_LIB "SynchroniLib32.lib")
        SET (SYNCHRONI_SDK_LINK_NAME_LIB "sensor32")
        SET (SYNCHRONI_SDK_LINK_NAME_DOT_LIB "sensor32.lib")
        SET (SYNCHRONI_SDK_ARCH "Win32")
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
    target_link_libraries (${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/mac/sensor.xcframework/macos-arm64_x86_64/sensor.framework)
elseif (UNIX)
    target_link_libraries (${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB})
else ()
    target_link_libraries (${SYNCHRONI_SDK_WRAPPER_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_DOT_LIB})
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
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll"
    )
endif (MSVC)

if (APPLE)
    add_custom_command (TARGET ${SYNCHRONI_SDK_WRAPPER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
    )
elseif (UNIX)
    add_custom_command (TARGET ${SYNCHRONI_SDK_WRAPPER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${SYNCHRONI_SDK_LINK_NAME_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${SYNCHRONI_SDK_LINK_NAME_LIB}"
    )
endif (APPLE)

if (MSVC)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        install (
            FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll
            DESTINATION lib
        )
    else (CMAKE_SIZEOF_VOID_P EQUAL 8)
        install (
            FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${SYNCHRONI_SDK_WRAPPER_NAME}.dll
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/windows/${SYNCHRONI_SDK_ARCH}/For$<CONFIG>/${SYNCHRONI_SDK_LINK_NAME_LIB}.dll
            DESTINATION lib
        )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (MSVC)

if (APPLE)
        install (
            FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}
            DESTINATION lib
        )
elseif (UNIX)
        install (
            FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/compiled/${SYNCHRONI_SDK_WRAPPER_COMPILED_NAME}
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/synchroni/lib/linux/${SYNCHRONI_SDK_ARCH}/For${CMAKE_BUILD_TYPE}/${SYNCHRONI_SDK_LINK_NAME_LIB}
            DESTINATION lib
        )
endif(APPLE)

install (
    TARGETS ${SYNCHRONI_SDK_WRAPPER_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)