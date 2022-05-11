cmake_minimum_required (VERSION 3.10)
project (BRAINFLOW_GET_DATA)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

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
configure_msvc_runtime()

find_package (
    brainflow CONFIG REQUIRED
)

add_executable (
    brainflow_get_data
    src/brainflow_get_data.cpp
)

target_include_directories (
    brainflow_get_data PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    brainflow_get_data PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

add_executable (
    markers
    src/markers.cpp
)

target_include_directories (
    markers PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    markers PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

add_executable (
    get_data_twice
    src/get_data_twice.cpp
)

target_include_directories (
    get_data_twice PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    get_data_twice PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)