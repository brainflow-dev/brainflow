add_custom_target (COPY_UNICORN_COMMAND)

if (MSVC)
    add_custom_command (TARGET COPY_UNICORN_COMMAND
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/Unicorn.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/Unicorn.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/Unicorn.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/Unicorn.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/Unicorn.dll"
    )
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    add_custom_command (TARGET COPY_UNICORN_COMMAND
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/libunicorn.so"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/linunicorn.so"
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)

if (MSVC)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll
        DESTINATION lib
    )
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
