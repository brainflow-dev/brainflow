if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
    elseif (UNIX)
        SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow")
    else ()
        SET (BRAINFLOW_CPP_BINDING_NAME "Brainflow32")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

add_library (
    ${BRAINFLOW_CPP_BINDING_NAME} STATIC
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/board_shim.cpp
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/ml_model.cpp
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/data_filter.cpp
)

target_include_directories (
    ${BRAINFLOW_CPP_BINDING_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/data_handler/inc
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/json
    ${CMAKE_HOME_DIRECTORY}/src/ml/inc
)

target_link_libraries (${BRAINFLOW_CPP_BINDING_NAME} PRIVATE ${BOARD_CONTROLLER_NAME} ${DATA_HANDLER_NAME} ${ML_MODULE_NAME})

install (
    FILES
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/data_filter.h
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/board_shim.h
    ${CMAKE_HOME_DIRECTORY}/cpp-package/src/inc/ml_model.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_array.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_exception.h
    DESTINATION inc
)

install (
    TARGETS ${BRAINFLOW_CPP_BINDING_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
