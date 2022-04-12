cmake_minimum_required (VERSION 3.10)
project (BRAINFLOW_ML_TEST)

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

###############
# eeg metrics #
###############
add_executable (
    eeg_metrics
    src/eeg_metrics.cpp
)
target_include_directories (
    eeg_metrics PUBLIC
    ${brainflow_INCLUDE_DIRS}
)
target_link_libraries (
    eeg_metrics PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

###########
# ci test #
###########
add_executable (
    eeg_metrics_ci
    src/eeg_metrics_ci.cpp
)
target_include_directories (
    eeg_metrics_ci PUBLIC
    ${brainflow_INCLUDE_DIRS}
)
target_link_libraries (
    eeg_metrics_ci PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

###########
# dyn lib #
###########
add_library (
    dyn_lib_test SHARED
    src/dyn_lib_test/dyn_lib_test.cpp
)

target_include_directories (
    dyn_lib_test PUBLIC
    ${brainflow_INCLUDE_DIRS}
)