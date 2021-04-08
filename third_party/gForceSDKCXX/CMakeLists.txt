cmake_minimum_required (VERSION 3.10)
project (gForceSDKWrapper)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

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

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (GFORCE_SDK_WRAPPER_NAME "gForceSDKWrapper")
    set (GFORCE_SDK_NAME "gforce64")
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (GFORCE_SDK_WRAPPER_NAME "gForceSDKWrapper32")
    set (GFORCE_SDK_NAME "gforce32")
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

set (GFORCE_WRAPPER_SRC
    ${PROJECT_SOURCE_DIR}/src/wrapper.cpp
    ${PROJECT_SOURCE_DIR}/../../src/utils/timestamp.cpp
)

add_library (
    ${GFORCE_SDK_WRAPPER_NAME} SHARED
    ${GFORCE_WRAPPER_SRC}
)

target_compile_definitions(${GFORCE_SDK_WRAPPER_NAME} PRIVATE -DUNICODE -D_UNICODE)

target_include_directories (
    ${GFORCE_SDK_WRAPPER_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/inc
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/..
    ${PROJECT_SOURCE_DIR}/../../src/utils/inc
)

find_library (GFORCE_SDK NAMES ${GFORCE_SDK_NAME} PATHS ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries (${GFORCE_SDK_WRAPPER_NAME} PRIVATE ${GFORCE_SDK})

set_target_properties (${GFORCE_SDK_WRAPPER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${GFORCE_SDK_WRAPPER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/compiled/$<CONFIG>/${GFORCE_SDK_WRAPPER_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${GFORCE_SDK_WRAPPER_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${GFORCE_SDK_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/${GFORCE_SDK_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${GFORCE_SDK_NAME}.dll"
    )
endif (MSVC)