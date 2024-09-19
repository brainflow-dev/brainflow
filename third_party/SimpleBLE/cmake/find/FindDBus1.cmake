# Attempt to load DBus1 as CONFIG silently
find_package(DBus1 CONFIG QUIET)

# If DBus1 was not found, try to find it as MODULE
if(NOT DBus1_FOUND)

    find_package(PkgConfig REQUIRED)
    pkg_search_module(PC_DBUS REQUIRED dbus-1)

    set(DBus1_LIBRARIES ${PC_DBUS_LIBRARIES})
    set(DBus1_INCLUDE_DIRS ${PC_DBUS_INCLUDE_DIRS})

    # setup imported target
    add_library(dbus-1 SHARED IMPORTED)
    set_property(TARGET dbus-1 APPEND PROPERTY IMPORTED_LOCATION ${PC_DBUS_LINK_LIBRARIES})
    set_property(TARGET dbus-1 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${DBus1_INCLUDE_DIRS})

    set(DBus1_FOUND ${PC_DBUS_FOUND})

endif()
