if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (ML_MODULE_NAME "MLModule")
    if (APPLE)
        SET (ML_MODULE_COMPILED_NAME "libMLModule.dylib")
    elseif (UNIX)
        SET (ML_MODULE_COMPILED_NAME "libMLModule.so")
    else ()
        SET (ML_MODULE_COMPILED_NAME "MLModule.dll")
        SET (ML_MODULE_COMPILED_NAME_DOT_LIB "MLModule.lib")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (ML_MODULE_NAME "MLModule")
        SET (ML_MODULE_COMPILED_NAME "libMLModule.dylib")
    elseif (UNIX)
        SET (ML_MODULE_NAME "MLModule")
        SET (ML_MODULE_COMPILED_NAME "libMLModule.so")
    else ()
        SET (ML_MODULE_NAME "MLModule32")
        SET (ML_MODULE_COMPILED_NAME "MLModule32.dll")
        SET (ML_MODULE_COMPILED_NAME_DOT_LIB "MLModule32.lib")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (ML_MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/ml_module.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dyn_lib_classifier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/onnx/onnx_classifier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/base_classifier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mindfulness_classifier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/generated/mindfulness_model.cpp
)

add_library (
    ${ML_MODULE_NAME} SHARED
    ${ML_MODULE_SRC}
)

target_include_directories (
    ${ML_MODULE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ml/onnx/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/onnxruntime/build/native/include
)

target_compile_definitions(${ML_MODULE_NAME} PRIVATE BRAINFLOW_VERSION=${BRAINFLOW_VERSION})

set_target_properties (${ML_MODULE_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/compiled
)

if (BUILD_ONNX)
    include (${CMAKE_CURRENT_LIST_DIR}/onnx/build.cmake)
endif (BUILD_ONNX)

if (USE_OPENMP)
    find_package (OpenMP)
    if (OpenMP_CXX_FOUND)
        target_link_libraries (${ML_MODULE_NAME} PRIVATE OpenMP::OpenMP_CXX)
    else (OpenMP_CXX_FOUND)
        message (FATAL_ERROR "ENABLE_OPENMP SET but no OpenMP found.")
    endif (OpenMP_CXX_FOUND)
endif (USE_OPENMP)

if (UNIX AND NOT ANDROID)
    target_link_libraries (${ML_MODULE_NAME} PRIVATE pthread dl)
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    find_library (log-lib log)
    target_link_libraries (${ML_MODULE_NAME} PRIVATE log)
endif (ANDROID)

if (MSVC)
    add_custom_command (TARGET ${ML_MODULE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc/ml_module.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/ml_module.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/$<CONFIG>/${ML_MODULE_COMPILED_NAME_DOT_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${ML_MODULE_COMPILED_NAME_DOT_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc/ml_module.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/ml_module.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/shared_export.h"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${ML_MODULE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/nodejs_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc/ml_module.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/ml_module.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/${ML_MODULE_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc/ml_module.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/ml_module.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/inc/shared_export_matlab.h" "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/inc/shared_export.h"
    )
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    add_custom_command (TARGET ${ML_MODULE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/compiled/${ML_MODULE_COMPILED_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/jniLibs/${ANDROID_ABI}/${ML_MODULE_COMPILED_NAME}"
    )
endif (ANDROID)

install (
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc/ml_module.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ml/inc/brainflow_model_params.h
    DESTINATION inc
)

install (
    TARGETS ${ML_MODULE_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
