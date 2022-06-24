include(FetchContent)

if (SIMPLEBLUEZ_VENDORIZE)

    # Load default parameters passed in through the command line.
    if(NOT SIMPLEBLUEZ_GIT_REPOSITORY)
        set(SIMPLEBLUEZ_GIT_REPOSITORY "https://github.com/OpenBluetoothToolbox/SimpleBluez.git")
    endif()
    if(NOT SIMPLEBLUEZ_GIT_TAG)
        set(SIMPLEBLUEZ_GIT_TAG "master") # TODO: Switch to the latest SimpleBluez release once done.
    endif()

    if(NOT SIMPLEBLUEZ_LOCAL_PATH)
        FetchContent_Declare(
            simplebluez
            GIT_REPOSITORY ${SIMPLEBLUEZ_GIT_REPOSITORY}
            GIT_TAG ${SIMPLEBLUEZ_GIT_TAG}
            GIT_SHALLOW YES
        )

        # Note that here we manually do what FetchContent_MakeAvailable() would do,
        # except to ensure that the dependency can also get what it needs, we add
        # custom logic between the FetchContent_Populate() and add_subdirectory()
        # calls.
        FetchContent_GetProperties(simplebluez)
        if(NOT simplebluez_POPULATED)
            FetchContent_Populate(simplebluez)
            list(APPEND CMAKE_MODULE_PATH "${simplebluez_SOURCE_DIR}/cmake/find")
            add_subdirectory("${simplebluez_SOURCE_DIR}" "${simplebluez_BINARY_DIR}")
        endif()

    else()
        list(APPEND CMAKE_MODULE_PATH "${SIMPLEBLUEZ_LOCAL_PATH}/cmake/find")
        add_subdirectory(${SIMPLEBLUEZ_LOCAL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/simplebluez)
    endif()

    # Because we are in a find module, we are solely responsible for resolution.
    # Setting this *_FOUND variable to a truthy value will signal to the calling
    # find_package() command that we were successful.
    # More relevant info regarding find modules and what variables they use can be
    # found in the documentation of find_package() and
    # https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
    set(simplebluez_FOUND 1)

endif()
