#pragma once

#include <string>
#include <windows.h>

#include <devguid.h>
#include <ntddser.h>
#include <setupapi.h>
#include <winerror.h>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "SetupAPI.lib")

static GUID GUID_FTDI_PORTS = {
    0x4d36e978, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18};

inline LONG set_dword_reg_key (
    HKEY key, const std::wstring &path, const std::wstring &value_name, DWORD value)
{
    HKEY final_key;
    LONG res = RegOpenKeyExW (key, path.c_str (), 0, KEY_SET_VALUE, &final_key);
    if (res == ERROR_SUCCESS)
    {
        res = RegSetValueExW (final_key, value_name.c_str (), 0, REG_DWORD,
            reinterpret_cast<LPBYTE> (&value), sizeof (DWORD));
        RegCloseKey (final_key);
    }
    return res;
}

inline LONG get_dword_reg_key (HKEY key, const std::wstring &path, const std::wstring &value_name,
    DWORD &value, DWORD default_value)
{
    value = default_value;
    HKEY final_key;
    LONG res = RegOpenKeyExW (key, path.c_str (), 0, KEY_READ, &final_key);
    if (res == ERROR_SUCCESS)
    {
        DWORD buffer_size (sizeof (DWORD));
        DWORD result (0);
        res = RegQueryValueExW (final_key, value_name.c_str (), 0, NULL,
            reinterpret_cast<LPBYTE> (&result), &buffer_size);
        if (res == ERROR_SUCCESS)
        {
            value = result;
        }
        RegCloseKey (final_key);
    }
    return res;
}

inline LONG get_bool_reg_key (HKEY key, const std::wstring &path, const std::wstring &value_name,
    bool &value, bool default_value)
{
    DWORD def_value ((default_value) ? 1 : 0);
    DWORD result (def_value);
    LONG res = get_dword_reg_key (key, path.c_str (), value_name.c_str (), result, def_value);
    if (res = ERROR_SUCCESS)
    {
        value = (result != 0) ? true : false;
    }
    return res;
}

inline LONG get_str_reg_key (HKEY key, const std::wstring &path, const std::wstring &value_name,
    std::wstring &value, const std::wstring &default_value)
{
    value = default_value;
    HKEY final_key;
    LONG res = RegOpenKeyExW (key, path.c_str (), 0, KEY_READ, &final_key);
    if (res == ERROR_SUCCESS)
    {
        WCHAR buffer[512];
        DWORD buffer_size = sizeof (buffer);
        LONG res = RegQueryValueExW (
            final_key, value_name.c_str (), 0, NULL, (LPBYTE)buffer, &buffer_size);
        if (res == ERROR_SUCCESS)
        {
            value = buffer;
        }
        RegCloseKey (final_key);
    }
    return res;
}

inline LONG restart_usb_device (std::string port_name)
{
    LONG res = ERROR_SUCCESS;
    bool need_wait = false;
    SP_DEVINFO_DATA device_info_data = {sizeof (device_info_data)};
    HDEVINFO dev_info = SetupDiGetClassDevs (&GUID_FTDI_PORTS, 0, 0, DIGCF_PRESENT);
    if (dev_info != INVALID_HANDLE_VALUE)
    {
        DWORD required_size = 0;
        for (int i = 0; SetupDiEnumDeviceInfo (dev_info, i, &device_info_data); i++)
        {
            DWORD data_t;
            char friendly_name[4096] = {0};
            DWORD buffer_size = 4096;
            DWORD req_bufsize = 0;
            // get device description information
            if (SetupDiGetDeviceRegistryPropertyA (dev_info, &device_info_data, SPDRP_FRIENDLYNAME,
                    &data_t, (PBYTE)friendly_name, buffer_size, &req_bufsize))
            {
                std::string name (friendly_name);
                if (name.find (port_name) != std::string::npos)
                {
                    // try to restart
                    if (SetupDiRestartDevices (dev_info, &device_info_data))
                    {
                        need_wait = true;
                    }
                    else
                    {
                        res = GetLastError ();
                    }
                }
            }
        }
    }
    else
    {
        res = GetLastError ();
    }
    if (need_wait)
    {
        Sleep (7000); // no idea about better option to wait after reboot
    }
    return res;
}

inline LONG set_ftdi_latency_in_registry (int latency_ms, std::string port_name)
{
    HKEY ftdi_parent_key;
    DWORD child_counter = 0;
    bool need_restart = false;
    LONG res = RegOpenKeyExW (HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS", 0,
        KEY_ENUMERATE_SUB_KEYS, &ftdi_parent_key);

    while (res == ERROR_SUCCESS)
    {
        WCHAR child_name[4096];
        DWORD child_size = sizeof (child_name);
        res = RegEnumKeyExW (
            ftdi_parent_key, child_counter, child_name, &child_size, NULL, NULL, NULL, NULL);
        if (res == ERROR_SUCCESS)
        {
            child_counter++;
            std::wstring full_name = std::wstring (L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS\\") +
                std::wstring (child_name) + std::wstring (L"\\0000\\Device Parameters");
            DWORD current_value = 0;
            if (get_dword_reg_key (HKEY_LOCAL_MACHINE, full_name, L"LatencyTimer", current_value,
                    1) == ERROR_SUCCESS)
            {
                if (current_value != latency_ms)
                {
                    if (set_dword_reg_key (HKEY_LOCAL_MACHINE, full_name, L"LatencyTimer",
                            latency_ms) == ERROR_SUCCESS)
                    {
                        need_restart = true;
                    }
                }
            }
        }
    }
    if (need_restart)
    {
        res = restart_usb_device (port_name);
    }
    else
    {
        res = ERROR_SUCCESS;
    }
    RegCloseKey (ftdi_parent_key);
    return res;
}