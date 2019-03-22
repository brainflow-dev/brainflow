cmake_minimum_required (VERSION 3.10)
project (brainflow)

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

if (MSVC)
    configure_msvc_runtime()
endif ()

include_directories (
    inc
    src/utils/
    src/board_controller/inc
    src/board_controller/openbci/inc
)

set (LIBRARY_SRC
    "src/utils/timestamp.cpp"
    "src/utils/data_buffer.cpp"
    "src/board_controller/openbci/openbci_board.cpp"
    "src/board_controller/openbci/cython.cpp"
    "src/board_controller/board_controller.cpp"
    "src/board_controller/board.cpp"
    )

add_library (
    BoardController SHARED
    ${LIBRARY_SRC}
)

set_target_properties (BoardController
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/compiled"
)

if (UNIX)
    target_link_libraries (BoardController -lpthread)
endif (UNIX)
