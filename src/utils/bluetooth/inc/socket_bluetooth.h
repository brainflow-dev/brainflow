#pragma once

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>
#endif

#include <stdlib.h>
#include <string.h>


enum class SocketBluetoothReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    CONNECT_ERROR = 3,
    WSA_ADDR_ERROR = 4,
    CONNECT_ERROR = 5,
    IOCTL_ERROR = 6
};

class SocketBluetooth
{

public:
    SocketBluetooth (std::string mac_addr, int port);
    ~SocketBluetooth ()
    {
        close ();
    }

    int connect ();
    int send (const char *data, int size);
    int recv (void *data, int size);
    int bytes_available ();
    int close ();

private:
    std::string mac_addr;
    int port;
#ifdef _WIN32
    SOCKET socket_bt;
#endif
};
