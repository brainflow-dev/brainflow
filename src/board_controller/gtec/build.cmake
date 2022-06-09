if (MSVC)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/rust_package/brainflow/lib/")
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp_package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/python_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia_package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/java_package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab_package/brainflow/lib/")
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
        ${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
