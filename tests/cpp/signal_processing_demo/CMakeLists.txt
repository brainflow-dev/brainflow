cmake_minimum_required (VERSION 3.10)
project (BRAINFLOW_SIGNAL_PROCESSING)

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

##############################
## Demo for read write file ##
##############################
add_executable (
    serialization
    src/serialization.cpp
)

target_include_directories (
    serialization PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    serialization PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

###########################
## Demo for downsampling ##
###########################
add_executable (
    downsampling
    src/downsampling.cpp
)

target_include_directories (
    downsampling PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    downsampling PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

###########################
## Demo for csp ##
###########################
add_executable (
    csp
    src/csp.cpp
)

target_include_directories (
    csp PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    csp PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

###########################
## Demo for windowing ##
###########################
add_executable (
    windowing
    src/windowing.cpp
)

target_include_directories (
    windowing PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    windowing PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

###############################
## Demo for signal filtering ##
###############################
add_executable (
    signal_filtering
    src/signal_filtering.cpp
)

target_include_directories (
    signal_filtering PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    signal_filtering PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

########################
## Demo for denoising ##
########################
add_executable (
    denoising
    src/denoising.cpp
)

target_include_directories (
    denoising PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    denoising PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

#########################
## Demo for transforms ##
#########################
add_executable (
    transforms
    src/transforms.cpp
)

target_include_directories (
    transforms PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    transforms PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

#########################
## Demo for band power ##
#########################
add_executable (
    band_power
    src/band_power.cpp
)

target_include_directories (
    band_power PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    band_power PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)

##########################################
## Demo for band powers across channels ##
##########################################
add_executable (
    band_power_all
    src/band_power_all.cpp
)

target_include_directories (
    band_power_all PUBLIC
    ${brainflow_INCLUDE_DIRS}
)

target_link_libraries (
    band_power_all PUBLIC
    # for some systems(ubuntu for example) order matters
    ${BrainflowPath}
    ${MLModulePath}
    ${DataHandlerPath}
    ${BoardControllerPath}
)
