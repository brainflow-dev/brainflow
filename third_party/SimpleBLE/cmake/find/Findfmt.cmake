# Read the documentation of FetchContent first!
# https://cmake.org/cmake/help/latest/module/FetchContent.html

include(FetchContent)

if (LIBFMT_VENDORIZE)

    # Load default parameters passed in through the command line.
    if(NOT LIBFMT_GIT_REPOSITORY)
        set(LIBFMT_GIT_REPOSITORY "https://github.com/fmtlib/fmt.git")
    endif()
    if(NOT LIBFMT_GIT_TAG)
        set(LIBFMT_GIT_TAG "b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9") # v8.1.1
    endif()

    if(NOT LIBFMT_LOCAL_PATH)
        # Library includes
        FetchContent_Declare(
            fmt
            GIT_REPOSITORY ${LIBFMT_GIT_REPOSITORY}
            GIT_TAG ${LIBFMT_GIT_TAG}
        )
        FetchContent_MakeAvailable(fmt)
    else()
        add_subdirectory(${LIBFMT_LOCAL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/libfmt)
    endif()

    # Because we are in a find module, we are solely responsible for resolution.
    # Setting this *_FOUND variable to a truthy value will signal to the calling
    # find_package() command that we were successful.
    # More relevant info regarding find modules and what variables they use can be
    # found in the documentation of find_package() and
    # https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
    set(fmt_FOUND 1)

endif()
