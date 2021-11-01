include (${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/third_party/wavelib/build.cmake)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (DATA_HANDLER_NAME "DataHandler")
    if (APPLE)
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.dylib")
    elseif (UNIX)
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.so")
    else ()
        SET (DATA_HANDLER_COMPILED_NAME "DataHandler.dll")
        SET (DATA_HANDLER_COMPILED_NAME_DOT_LIB "DataHandler.lib")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (DATA_HANDLER_NAME "DataHandler")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.dylib")
    elseif (UNIX)
        SET (DATA_HANDLER_NAME "DataHandler")
        SET (DATA_HANDLER_COMPILED_NAME "libDataHandler.so")
    else ()
        SET (DATA_HANDLER_NAME "DataHandler32")
        SET (DATA_HANDLER_COMPILED_NAME "DataHandler32.dll")
        SET (DATA_HANDLER_COMPILED_NAME_DOT_LIB "DataHandler32.lib")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (DATA_HANDLER_SRC
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/data_handler.cpp
)

add_library (
    ${DATA_HANDLER_NAME} SHARED
    ${DATA_HANDLER_SRC}
)

target_include_directories (
    ${DATA_HANDLER_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/third_party/
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/DSPFilters/include
    ${CMAKE_HOME_DIRECTORY}/third_party/wavelib/header
    ${CMAKE_HOME_DIRECTORY}/third_party/fft/src
)

set_target_properties (${DATA_HANDLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (USE_OPENMP)
    find_package (OpenMP)
    if (OpenMP_CXX_FOUND)
        target_link_libraries (${DATA_HANDLER_NAME} PRIVATE OpenMP::OpenMP_CXX)
    else (OpenMP_CXX_FOUND)
        message (FATAL_ERROR "ENABLE_OPENMP SET but no OpenMP found.")
    endif (OpenMP_CXX_FOUND)
endif (USE_OPENMP)

if (UNIX AND NOT ANDROID)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE ${DSPFILTERS} ${WAVELIB} pthread dl)
else (UNIX AND NOT ANDROID)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE ${DSPFILTERS} ${WAVELIB})
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    find_library (log-lib log)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE  log)
endif (ANDROID)

if (MSVC)
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/data_handler.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME_DOT_LIB}" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME_DOT_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/inc/data_handler.h"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/data_handler.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/inc/data_handler.h"
    )
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/tools/jniLibs/${ANDROID_ABI}/${DATA_HANDLER_COMPILED_NAME}"
    )
endif (ANDROID)

install (
    FILES
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/inc/data_handler.h
    DESTINATION inc
)

install (
    TARGETS ${DSPFILTERS} ${WAVELIB} ${DATA_HANDLER_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
