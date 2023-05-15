if (MSVC)
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/rust_package/brainflow/lib/")
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/matlab_package/brainflow/lib/")
endif (UNIX AND NOT APPLE AND NOT ANDROID)

if (MSVC)
    install (
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/Unicorn.dll
        DESTINATION lib
    )
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    install (
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn.so
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unicorn/lib/libunicorn_raspberry.so
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
