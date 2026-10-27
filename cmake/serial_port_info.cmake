# Native metadata enumeration only; no new redistributable library.
function (link_serial_port_info target)
    if (WIN32)
        target_link_libraries (${target} PRIVATE setupapi cfgmgr32 advapi32)
    elseif (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_link_libraries (${target} PRIVATE "-framework IOKit" "-framework CoreFoundation")
    endif ()
endfunction ()
