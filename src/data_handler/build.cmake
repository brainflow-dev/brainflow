include (${CMAKE_CURRENT_SOURCE_DIR}/third_party/DSPFilters/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/third_party/wavelib/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/third_party/kissfft/build.cmake)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/data_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/fastica.cpp
)

add_library (
    ${DATA_HANDLER_NAME} SHARED
    ${DATA_HANDLER_SRC}
)

target_include_directories (
    ${DATA_HANDLER_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board_controller/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/DSPFilters/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/wavelib/header
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/kissfft
)

target_compile_definitions(${DATA_HANDLER_NAME} PRIVATE BRAINFLOW_VERSION=${BRAINFLOW_VERSION})

set_target_properties (${DATA_HANDLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
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
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE ${DSPFILTERS} ${WAVELIB} kissfft pthread dl)
else (UNIX AND NOT ANDROID)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE ${DSPFILTERS} ${WAVELIB} kissfft)
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    find_library (log-lib log)
    target_link_libraries (${DATA_HANDLER_NAME} PRIVATE  log)
endif (ANDROID)

if (MSVC)
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/inc/data_handler.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/data_handler.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${DATA_HANDLER_COMPILED_NAME_DOT_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME_DOT_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/inc/data_handler.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/data_handler.h"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/inc/data_handler.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/data_handler.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${DATA_HANDLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/inc/data_handler.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/data_handler.h"
    )
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    add_custom_command (TARGET ${DATA_HANDLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${DATA_HANDLER_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/jniLibs/${ANDROID_ABI}/${DATA_HANDLER_COMPILED_NAME}"
    )
endif (ANDROID)

install (
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler/inc/data_handler.h
    DESTINATION inc
)

install (
    TARGETS ${DSPFILTERS} ${WAVELIB} kissfft ${DATA_HANDLER_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
