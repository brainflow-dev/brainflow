#pragma once

#include "serial.h"

// Internal helpers shared by native enumeration and hardware-independent tests.
namespace serial_port_info
{
    int parse_usb_id (const std::string &value);
    bool parse_windows_usb_instance (
        const std::string &instance, bool unique_id, SerialPortInfo &port);

#if defined(__linux__) && !defined(__ANDROID__)
    int enumerate_linux (std::vector<SerialPortInfo> &ports, const std::string &sysfs_root,
        const std::string &dev_root);
#endif
}
