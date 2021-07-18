if (MSVC)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/Unicorn.dll" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/unicorn/lib/libunicorn_raspberry.so" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
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
