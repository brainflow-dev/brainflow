if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (BRAINALIVE_NATIVE_LIB "BrainAlive_Native_Lib")
    if (APPLE)
        SET (BRAINALIVE_NATIVE_LIB_NAME "BrainAlive_Native_Lib.dylib")
    elseif (UNIX)
        SET (BRAINALIVE_NATIVE_LIB_NAME "BrainAlive_Native_Lib.so")
    else ()
  	SET (BRAINALIVE_NATIVE_LIB_NAME "BrainAlive_Native_Lib.dll")
	SET (BRAINALIVE_LIB_NAME "BrainAlive_Lib.dll")
	SET (BRAINALIVE_NATIVE_LIB_PATH "${CMAKE_HOME_DIRECTORY}/src/board_controller/BrainAlive/inc/BrainAlive_Lib/Release/BrainAlive_Native_Lib.dll") 
	SET (BRAINALIVE_LIB_PATH "${CMAKE_HOME_DIRECTORY}/src/board_controller/BrainAlive/inc/BrainAlive_Lib/Release/BrainAlive_Lib.dll") 
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    MESSAGE ("32 bits compiler detected")
    SET (PLATFORM_ACH "X86")
     if (APPLE)
        SET (BRAINALIVE_NATIVE_LIB_NAME "BrainAlive_Native_Lib.dylib")
    elseif (UNIX)
        SET (BRAINALIVE_NATIVE_LIB_NAME "BrainAlive_Native_Lib.so")
    else ()
        SET (BRAINALIVE_NATIVE_LIB_NAME "BrainAlive_Native_Lib32.dll")
	    SET (BRAINALIVE_NATIVE_LIB_PATH "${CMAKE_HOME_DIRECTORY}/src/board_controller/BrainAlive/inc/BrainAlive_Lib/Release/BrainAlive_Native_Lib.dll")
	    SET (BRAINALIVE_LIB_PATH "${CMAKE_HOME_DIRECTORY}/src/board_controller/BrainAlive/inc/BrainAlive_Lib/Release/BrainAlive_Lib.dll") 
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

target_include_directories (${BRAINALIVE_NATIVE_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/src/board_controller/BrainAlive/inc/BrainALive_Lib/BrainAlive_Native_Lib>
)

if (MSVC)
    add_custom_command (TARGET ${BRAINALIVE_NATIVE_LIB} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BRAINALIVE_NATIVE_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BRAINALIVE_NATIVE_LIB_NAME}"
	    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BRAINALIVE_NATIVE_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BRAINALIVE_NATIVE_LIB_NAME}"
	    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BRAINALIVE_NATIVE_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${BRAINALIVE_NATIVE_LIB_NAME}"
 	    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BRAINALIVE_NATIVE_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BRAINALIVE_NATIVE_LIB_NAME}"
	    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BRAINALIVE_NATIVE_LIB_PATH}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${BRAINALIVE_NATIVE_LIB_NAME}"
    )
endif (MSVC)

install (
    TARGETS ${BRAINALIVE_NATIVE_LIB}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
