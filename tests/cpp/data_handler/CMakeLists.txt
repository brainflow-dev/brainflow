cmake_minimum_required (VERSION 3.10)
project (BRAINFLOW_DATA_HANDLER_TEST)

set (CMAKE_CXX_STANDARD 14)
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

# Test signal processing with state same as all at once
add_executable (
    signal_filtering_with_state
    src/signal_filtering.cpp
)

target_include_directories (
    signal_filtering_with_state PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    signal_filtering_with_state PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

# Test signal processing with state, using C API, same as all at once
add_executable (
    signal_filtering_c_with_state
    src/signal_filtering_c_api.cpp
)

target_include_directories (
    signal_filtering_c_with_state PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    signal_filtering_c_with_state PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)
