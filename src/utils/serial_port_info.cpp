#include <algorithm>
#include <cctype>
#include <string>

#include "serial_port_info.h"

#ifdef _WIN32
#include <cfgmgr32.h>
#include <setupapi.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#endif
#elif defined(__linux__) && !defined(__ANDROID__)
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#endif

int serial_port_info::parse_usb_id (const std::string &value)
{
    if (value.size () != 4)
    {
        return -1;
    }
    int result = 0;
    for (char c : value)
    {
        c = (char)std::tolower ((unsigned char)c);
        int digit = (c >= '0' && c <= '9') ? c - '0' : ((c >= 'a' && c <= 'f') ? c - 'a' + 10 : -1);
        if (digit < 0)
        {
            return -1;
        }
        result = result * 16 + digit;
    }
    return result;
}

bool serial_port_info::parse_windows_usb_instance (
    const std::string &instance, bool unique_id, SerialPortInfo &port)
{
    std::string id = instance;
    std::transform (id.begin (), id.end (), id.begin (),
        [] (unsigned char c) { return (char)std::toupper (c); });
    // Walk past composite interfaces and FTDIBUS children to the physical USB device.
    if (id.size () < 23 || id.compare (0, 8, "USB\\VID_") != 0 || id.compare (12, 5, "&PID_") != 0)
    {
        return false;
    }
    size_t separator = id.find ('\\', 8);
    if (separator != 21 || separator + 1 == id.size ())
    {
        return false;
    }
    int vid = parse_usb_id (id.substr (8, 4));
    int pid = parse_usb_id (id.substr (17, 4));
    if (vid < 0 || pid < 0)
    {
        return false;
    }
    port.vid = vid;
    port.pid = pid;
    // A location-generated instance suffix is not a USB serial number.
    port.serial_number = unique_id ? instance.substr (separator + 1) : "";
    return true;
}

#ifdef _WIN32
static int enumerate_native (std::vector<SerialPortInfo> &ports)
{
    // GUID_DEVCLASS_PORTS, avoiding a dependency on uuid.lib.
    const GUID ports_class = {
        0x4d36e978, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18}};
    HDEVINFO devices = SetupDiGetClassDevsA (&ports_class, NULL, NULL, DIGCF_PRESENT);
    if (devices == INVALID_HANDLE_VALUE)
    {
        return SerialExitCodes::ENUMERATION_ERROR;
    }
    int result = SerialExitCodes::OK;
    for (DWORD index = 0;; index++)
    {
        SP_DEVINFO_DATA device = {};
        device.cbSize = sizeof (device);
        if (!SetupDiEnumDeviceInfo (devices, index, &device))
        {
            if (GetLastError () != ERROR_NO_MORE_ITEMS)
            {
                result = SerialExitCodes::ENUMERATION_ERROR;
            }
            break;
        }
        HKEY key = SetupDiOpenDevRegKey (
            devices, &device, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
        if (key == INVALID_HANDLE_VALUE)
        {
            continue;
        }
        char name[256] = {};
        DWORD size = sizeof (name) - 1;
        DWORD type = 0;
        LONG status = RegQueryValueExA (key, "PortName", NULL, &type, (LPBYTE)name, &size);
        RegCloseKey (key);
        if (status != ERROR_SUCCESS || type != REG_SZ)
        {
            continue;
        }
        SerialPortInfo port;
        port.port = name;
        if (port.port.size () < 4 || port.port.compare (0, 3, "COM") != 0 ||
            port.port.find_first_not_of ("0123456789", 3) != std::string::npos)
        {
            continue;
        }
        DEVINST node = device.DevInst;
        for (int depth = 0; depth < 32; depth++)
        {
            char instance[MAX_DEVICE_ID_LEN] = {};
            if (CM_Get_Device_IDA (node, instance, MAX_DEVICE_ID_LEN, 0) == CR_SUCCESS)
            {
                ULONG capabilities = 0;
                ULONG bytes = sizeof (capabilities);
                CM_Get_DevNode_Registry_PropertyA (
                    node, CM_DRP_CAPABILITIES, NULL, &capabilities, &bytes, 0);
                if (serial_port_info::parse_windows_usb_instance (
                        instance, (capabilities & CM_DEVCAP_UNIQUEID) != 0, port))
                {
                    ports.push_back (port);
                    break;
                }
            }
            DEVINST parent;
            if (CM_Get_Parent (&parent, node, 0) != CR_SUCCESS)
            {
                break;
            }
            node = parent;
        }
    }
    SetupDiDestroyDeviceInfoList (devices);
    return result;
}
#elif defined(__APPLE__) && TARGET_OS_OSX
static std::string string_property (io_registry_entry_t entry, CFStringRef key)
{
    std::string result;
    CFTypeRef value = IORegistryEntryCreateCFProperty (entry, key, kCFAllocatorDefault, 0);
    if (value)
    {
        char buffer[1024];
        if (CFGetTypeID (value) == CFStringGetTypeID () &&
            CFStringGetCString ((CFStringRef)value, buffer, sizeof (buffer), kCFStringEncodingUTF8))
        {
            result = buffer;
        }
        CFRelease (value);
    }
    return result;
}

static int number_property (io_registry_entry_t entry, CFStringRef key)
{
    int result = -1;
    CFTypeRef value = IORegistryEntryCreateCFProperty (entry, key, kCFAllocatorDefault, 0);
    if (value)
    {
        if (CFGetTypeID (value) == CFNumberGetTypeID ())
        {
            CFNumberGetValue ((CFNumberRef)value, kCFNumberIntType, &result);
        }
        CFRelease (value);
    }
    return result;
}

static int enumerate_native (std::vector<SerialPortInfo> &ports)
{
    CFMutableDictionaryRef matching = IOServiceMatching (kIOSerialBSDServiceValue);
    if (!matching)
    {
        return SerialExitCodes::ENUMERATION_ERROR;
    }
    io_iterator_t iterator = IO_OBJECT_NULL;
    // Zero is the default IOKit port, also on older supported macOS SDKs.
    if (IOServiceGetMatchingServices (0, matching, &iterator) != KERN_SUCCESS)
    {
        return SerialExitCodes::ENUMERATION_ERROR;
    }
    io_registry_entry_t node;
    while ((node = IOIteratorNext (iterator)) != IO_OBJECT_NULL)
    {
        SerialPortInfo port;
        // Only the callout path: /dev/tty.* would duplicate the same physical device.
        port.port = string_property (node, CFSTR (kIOCalloutDeviceKey));
        for (int depth = 0; depth < 32; depth++)
        {
            if (IOObjectConformsTo (node, "IOUSBHostDevice") ||
                IOObjectConformsTo (node, "IOUSBDevice"))
            {
                port.vid = number_property (node, CFSTR ("idVendor"));
                port.pid = number_property (node, CFSTR ("idProduct"));
                port.serial_number = string_property (node, CFSTR ("USB Serial Number"));
                if (!port.port.empty () && port.vid >= 0 && port.pid >= 0)
                {
                    ports.push_back (port);
                }
                break;
            }
            io_registry_entry_t parent;
            if (IORegistryEntryGetParentEntry (node, kIOServicePlane, &parent) != KERN_SUCCESS)
            {
                break;
            }
            IOObjectRelease (node);
            node = parent;
        }
        IOObjectRelease (node);
    }
    IOObjectRelease (iterator);
    return SerialExitCodes::OK;
}
#elif defined(__linux__) && !defined(__ANDROID__)
static std::string read_attribute (const std::string &path)
{
    std::ifstream input (path);
    std::string value;
    std::getline (input, value);
    return value;
}

int serial_port_info::enumerate_linux (
    std::vector<SerialPortInfo> &ports, const std::string &sysfs_root, const std::string &dev_root)
{
    ports.clear ();
    DIR *directory = opendir (sysfs_root.c_str ());
    if (!directory)
    {
        return SerialExitCodes::ENUMERATION_ERROR;
    }
    int result = SerialExitCodes::OK;
    while (true)
    {
        errno = 0;
        struct dirent *entry = readdir (directory);
        if (!entry)
        {
            result = errno == 0 ? SerialExitCodes::OK : SerialExitCodes::ENUMERATION_ERROR;
            break;
        }
        if (entry->d_name[0] == '.')
        {
            continue;
        }
        SerialPortInfo port;
        port.port = dev_root + "/" + entry->d_name;
        struct stat info;
        if (stat (port.port.c_str (), &info) != 0)
        {
            continue;
        }
        std::string device = sysfs_root + "/" + entry->d_name + "/device";
        char *resolved = realpath (device.c_str (), NULL);
        if (!resolved)
        {
            continue;
        }
        device = resolved;
        free (resolved);
        while (!device.empty ())
        {
            std::string vid = read_attribute (device + "/idVendor");
            std::string pid = read_attribute (device + "/idProduct");
            if (!vid.empty () || !pid.empty ())
            {
                port.vid = parse_usb_id (vid);
                port.pid = parse_usb_id (pid);
                port.serial_number = read_attribute (device + "/serial");
                if (port.vid >= 0 && port.pid >= 0)
                {
                    ports.push_back (port);
                }
                break;
            }
            device = device.substr (0, device.find_last_of ('/'));
        }
    }
    closedir (directory);
    if (result != SerialExitCodes::OK)
    {
        ports.clear ();
    }
    return result;
}

static int enumerate_native (std::vector<SerialPortInfo> &ports)
{
    return serial_port_info::enumerate_linux (ports, "/sys/class/tty", "/dev");
}
#else
static int enumerate_native (std::vector<SerialPortInfo> &ports)
{
    return SerialExitCodes::NO_SYSTEM_HEADERS_FOUND_ERROR;
}
#endif

int Serial::get_port_info (std::vector<SerialPortInfo> &ports)
{
    ports.clear ();
    int result = enumerate_native (ports);
    if (result != SerialExitCodes::OK)
    {
        ports.clear (); // Never select from an incomplete enumeration after an error.
        return result;
    }
    std::sort (ports.begin (), ports.end (),
        [] (const SerialPortInfo &a, const SerialPortInfo &b) { return a.port < b.port; });
    ports.erase (
        std::unique (ports.begin (), ports.end (),
            [] (const SerialPortInfo &a, const SerialPortInfo &b) { return a.port == b.port; }),
        ports.end ());
    return SerialExitCodes::OK;
}
