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

if (MSVC)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/${NEUROSDK_LIB_NAME}.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
endif (MSVC)
if (APPLE)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/lib/lib${NEUROSDK_LIB_NAME}.dylib" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
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