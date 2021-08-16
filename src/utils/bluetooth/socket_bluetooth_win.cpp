#include "bluetooth_functions.h"
#include "bluetooth_types.h"
#include "socket_bluetooth.h"

#include <bluetoothapis.h>
#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>

#undef UNICODE
#undef _UNICODE

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "bthprops.lib")


std::string uchar2hex (unsigned char inchar)
{
    std::ostringstream oss (std::ostringstream::out);
    oss << std::setw (2) << std::setfill ('0') << std::hex << (int)(inchar);
    return oss.str ();
}

SocketBluetooth::SocketBluetooth (std::string mac_addr, int port)
{
    this->mac_addr = mac_addr;
    this->port = port;
    socket_bt = INVALID_SOCKET;
}

int SocketBluetooth::connect ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketBluetoothReturnCodes::WSA_STARTUP_ERROR;
    }

    int status = SOCKET_ERROR;
    socket_bt = socket (AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (socket_bt == INVALID_SOCKET)
    {
        return (int)SocketBluetoothReturnCodes::CREATE_SOCKET_ERROR;
    }
    SOCKADDR_BTH addr = {0};
    int addr_size = sizeof (SOCKADDR_BTH);
    char mac_addr_c[40];
    strncpy (mac_addr_c, mac_addr.c_str (), 40);
    status = WSAStringToAddress (mac_addr_c, AF_BTH, NULL, (LPSOCKADDR)&addr, &addr_size);
    if (status == SOCKET_ERROR)
    {
        close ();
        return (int)SocketBluetoothReturnCodes::WSA_ADDR_ERROR;
    }
    addr.port = this->port;

    status = ::connect (socket_bt, (LPSOCKADDR)&addr, addr_size);
    if (status == SOCKET_ERROR)
    {
        close ();
        return (int)SocketBluetoothReturnCodes::CONNECT_ERROR;
    }
    unsigned long enable_non_blocking = 1;
    status = ioctlsocket (socket_bt, FIONBIO, &enable_non_blocking);

    if (status == SOCKET_ERROR)
    {
        close ();
        return (int)SocketBluetoothReturnCodes::IOCTL_ERROR;
    }
    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

int SocketBluetooth::send (const char *data, int size)
{
    if (socket_bt == INVALID_SOCKET)
    {
        return -1;
    }
    int res = ::send (socket_bt, data, size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int SocketBluetooth::recv (char *data, int size)
{
    if (socket_bt == INVALID_SOCKET)
    {
        return -1;
    }
    // waiting for exact amount of bytes
    int e = bytes_available ();
    if (e < size)
    {
        return 0;
    }
    fd_set set;
    FD_ZERO (&set);
    FD_SET (socket_bt, &set);

    int res = -1;

    timeval timeout {0, 0};
    if (select (1, &set, nullptr, nullptr, &timeout) >= 0)
    {
        if (FD_ISSET (socket_bt, &set))
        {
            res = ::recv (socket_bt, data, size, 0);
        }
    }
    return res;
}

int SocketBluetooth::bytes_available ()
{
    if (socket_bt == INVALID_SOCKET)
    {
        return -1;
    }
    u_long count;
    ioctlsocket (socket_bt, FIONREAD, &count);
    return count;
}

int SocketBluetooth::close ()
{
    closesocket (socket_bt);
    socket_bt = INVALID_SOCKET;
    WSACleanup ();
    return (int)SocketBluetoothReturnCodes::STATUS_OK;
}

std::pair<std::string, int> SocketBluetooth::discover (char *selector)
{
    HBLUETOOTH_DEVICE_FIND founded_device;
    BLUETOOTH_DEVICE_INFO device_info;
    device_info.dwSize = sizeof (device_info);

    BLUETOOTH_DEVICE_SEARCH_PARAMS search_criteria;
    search_criteria.dwSize = sizeof (BLUETOOTH_DEVICE_SEARCH_PARAMS);
    search_criteria.fReturnAuthenticated = TRUE;
    search_criteria.fReturnRemembered = FALSE;
    search_criteria.fReturnConnected = FALSE;
    search_criteria.fReturnUnknown = FALSE;
    search_criteria.fIssueInquiry = FALSE;
    search_criteria.cTimeoutMultiplier = 0;

    founded_device = BluetoothFindFirstDevice (&search_criteria, &device_info);
    if (founded_device == NULL)
    {
        return std::make_pair<std::string, int> (
            "", (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
    }

    do
    {
        std::wstring device_name_w = std::wstring (device_info.szName);
        // setup converter
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        // use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
        std::string device_name = converter.to_bytes (device_name_w);
        if (device_name.find (std::string (selector)) != device_name.npos)
        {
            std::string address_string = "";
            BLUETOOTH_ADDRESS addr = device_info.Address;
            for (int i = 0; i < 6; i++)
            {
                address_string += uchar2hex (addr.rgBytes[5 - i]);
                if (i != 5)
                {
                    address_string += ":";
                }
            }
            std::pair<std::string, int> res = std::pair<std::string, int> (
                address_string, (int)SocketBluetoothReturnCodes::STATUS_OK);
            return res;
        }

    } while (BluetoothFindNextDevice (founded_device, &device_info));
    return std::make_pair<std::string, int> (
        "", (int)SocketBluetoothReturnCodes::DEVICE_IS_NOT_CREATED_ERROR);
}