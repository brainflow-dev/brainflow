if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (UNIX AND NOT APPLE AND NOT ANDROID)
        SET (ANT_LIB_NAME "libeego-SDK.so")
    endif (UNIX AND NOT APPLE AND NOT ANDROID)
    if (MSVC)
        SET (ANT_LIB_NAME "eego-SDK.dll")
    endif (MSVC)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (UNIX AND NOT APPLE)
        message (WARNING "32 bit lib is not available for linux")
        SET (ANT_LIB_NAME "libeego-SDK.so") 
    endif (UNIX AND NOT APPLE)
    if (MSVC)
        SET (ANT_LIB_NAME "eego-SDK32.dll")
    endif (MSVC)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

if (MSVC OR UNIX AND NOT APPLE AND NOT ANDROID)
    SET (BOARD_CONTROLLER_SRC ${BOARD_CONTROLLER_SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/eemagine/sdk/wrapper.cc)
endif (MSVC OR UNIX AND NOT APPLE AND NOT ANDROID)

if (MSVC)
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/")
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/")
endif (UNIX AND NOT APPLE AND NOT ANDROID)

if (MSVC)
    install (
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/windows/${ANT_LIB_NAME}
        DESTINATION lib
    )
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    install (
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ant_neuro/linux/${ANT_LIB_NAME}
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
