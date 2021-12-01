if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (ONNX_LIB_NAME "brainflow_onnx")
    if (APPLE)
        SET (ONNX_LIB_COMPILED_NAME "libbrainflow_onnx.dylib")
    elseif (UNIX)
        SET (ONNX_LIB_COMPILED_NAME "libbrainflow_onnx.so")
    else ()
        SET (ONNX_LIB_COMPILED_NAME "brainflow_onnx.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (ONNX_LIB_NAME "brainflow_onnx")
        SET (ONNX_LIB_COMPILED_NAME "libbrainflow_onnx.dylib")
    elseif (UNIX)
        SET (ONNX_LIB_NAME "brainflow_onnx")
        SET (ONNX_LIB_COMPILED_NAME "libbrainflow_onnx.so")
    else ()
        SET (ONNX_LIB_NAME "brainflow_onnx32")
        SET (ONNX_LIB_COMPILED_NAME "brainflow_onnx32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

if (APPLE)
    SET (ONNXRUNTIME_PATH ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/osx.10.14-x64/native/libonnxruntime.dylib)
    find_library (ONNXRUNTIME NAMES libonnxruntime onnxruntime PATHS ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/osx.10.14-x64/native)
elseif (MSVC)
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET (ONNXRUNTIME_PATH ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-arm64/native/onnxruntime_arm64.dll)
            find_library (ONNXRUNTIME NAMES onnxruntime PATHS ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-arm64/native)
        else (CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET (ONNXRUNTIME_PATH ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-arm6/native/onnxruntime_arm.dll)
            find_library (ONNXRUNTIME NAMES onnxruntime PATHS ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-arm/native)
        endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    else (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET (ONNXRUNTIME_PATH ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-x64/native/onnxruntime_x64.dll)
            find_library (ONNXRUNTIME NAMES onnxruntime PATHS ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-x64/native)
        else (CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET (ONNXRUNTIME_PATH ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-x86/native/onnxruntime_x86.dll)
            find_library (ONNXRUNTIME NAMES onnxruntime PATHS ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/win-x86/native)
        endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    endif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
elseif (UNIX AND NOT ANDROID)
    SET (ONNXRUNTIME_PATH ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/linux-x64/native/libonnxruntime.so)
    find_library (ONNXRUNTIME NAMES libonnxruntime onnxruntime PATHS ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/runtimes/linux-x64/native)
endif (APPLE)

SET (ONNX_LIB_SRC
    ${CMAKE_HOME_DIRECTORY}/src/ml/onnx/brainflow_onnx_lib.cpp
)

add_library (
    ${ONNX_LIB_NAME} SHARED
    ${ONNX_LIB_SRC}
)

target_include_directories (
    ${ONNX_LIB_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/src/ml/onnx/inc
    ${CMAKE_HOME_DIRECTORY}/src/ml/inc
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/onnxruntime/build/native/include
)

set_target_properties (${ONNX_LIB_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (MSVC)
    add_custom_command (TARGET ${ONNX_LIB_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${ONNX_LIB_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${ONNX_LIB_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/lib/${ONNX_LIB_COMPILED_NAME}"
    )
endif (UNIX AND NOT ANDROID)

if (NOT ANDROID)
    file (COPY ${ONNXRUNTIME_PATH} DESTINATION ${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/)
    file (COPY ${ONNXRUNTIME_PATH} DESTINATION ${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/)
    file (COPY ${ONNXRUNTIME_PATH} DESTINATION ${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/)
    file (COPY ${ONNXRUNTIME_PATH} DESTINATION ${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/)
    file (COPY ${ONNXRUNTIME_PATH} DESTINATION ${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/)
    file (COPY ${ONNXRUNTIME_PATH} DESTINATION ${CMAKE_HOME_DIRECTORY}/rust-package/brainflow/lib/)
endif (NOT ANDROID)

if (NOT ANDROID)
    target_link_libraries (${ONNX_LIB_NAME} PRIVATE ${ONNXRUNTIME})
endif (NOT ANDROID)

install (
    TARGETS ${ONNX_LIB_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
