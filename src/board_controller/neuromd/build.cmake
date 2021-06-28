include (${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/brainbit_bglib/build.cmake)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (NEUROSDK_LIB_NAME "neurosdk-shared")
    endif (APPLE)
    if (MSVC)
        SET (NEUROSDK_LIB_NAME "neurosdk-x64")
    endif (MSVC)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (MSVC)
        SET (NEUROSDK_LIB_NAME "neurosdk-x86")
    endif (MSVC)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

add_custom_target (COPY_NEUROSDK_COMMAND)

if (MSVC)
    add_custom_command (TARGET COPY_NEUROSDK_COMMAND
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${NEUROSDK_LIB_NAME}.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${NEUROSDK_LIB_NAME}.dll"
    )
endif (MSVC)
if (APPLE)
    add_custom_command (TARGET COPY_NEUROSDK_COMMAND
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/lib${NEUROSDK_LIB_NAME}.dylib"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/lib${NEUROSDK_LIB_NAME}.dylib"
    )
endif (APPLE)

if (MSVC)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll
        DESTINATION lib
    )
endif (MSVC)

if (APPLE)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib
        DESTINATION lib
    )
endif (APPLE)