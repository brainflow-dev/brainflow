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
    set (GFORCE_SDK_NAMES "gforce64")
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("64 bits compiler detected")
    SET (GFORCE_SDK_WRAPPER_NAME "gForceSDKWrapper32")
    set (GFORCE_SDK_NAMES "gforce32")
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

set (GFORCE_WRAPPER_SRC
    ${CMAKE_HOME_DIRECTORY}/src/wrapper.cpp
)

add_library (
    ${GFORCE_SDK_WRAPPER_NAME} SHARED
    ${GFORCE_WRAPPER_SRC}
)

target_include_directories (
    ${GFORCE_SDK_WRAPPER_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/src/inc
    ${CMAKE_HOME_DIRECTORY}/inc
)

find_library (GFORCE_SDK NAMES ${GFORCE_SDK_NAMES} PATHS ${CMAKE_HOME_DIRECTORY}/lib)
target_link_libraries (${GFORCE_SDK_WRAPPER_NAME} PRIVATE ${GFORCE_SDK})