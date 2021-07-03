if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (GFORCE_SDK_WRAPPER_NAME "gForceSDKWrapper")
    SET (GFORCE_SDK_NAME "gforce64")
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (GFORCE_SDK_WRAPPER_NAME "gForceSDKWrapper32")
    SET (GFORCE_SDK_NAME "gforce32")
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (GFORCE_WRAPPER_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/wrapper.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/timestamp.cpp
)

add_library (
    ${GFORCE_SDK_WRAPPER_NAME} SHARED
    ${GFORCE_WRAPPER_SRC}
)

target_compile_definitions(${GFORCE_SDK_WRAPPER_NAME} PRIVATE -DUNICODE -D_UNICODE)

target_include_directories (
    ${GFORCE_SDK_WRAPPER_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/json
)

find_library (GFORCE_SDK NAMES ${GFORCE_SDK_NAME} PATHS ${CMAKE_CURRENT_LIST_DIR}/lib)
target_link_libraries (${GFORCE_SDK_WRAPPER_NAME} PRIVATE ${GFORCE_SDK})

set_target_properties (${GFORCE_SDK_WRAPPER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${GFORCE_SDK_WRAPPER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${GFORCE_SDK_NAME}.dll"
    )
endif (MSVC)

if (MSVC)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        install (
            FILES
            ${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/gForceSDKWrapper.dll
            ${CMAKE_HOME_DIRECTORY}/third_party/gForceSDKCXX/lib/gforce64.dll
            DESTINATION lib
        )
    else (CMAKE_SIZEOF_VOID_P EQUAL 8)
        install (
            FILES
            ${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/gForceSDKWrapper32.dll
            ${CMAKE_HOME_DIRECTORY}/third_party/gForceSDKCXX/lib/gforce32.dll
            DESTINATION lib
        )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (MSVC)
