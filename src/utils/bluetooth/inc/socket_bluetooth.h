#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2bth.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <string>


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
    int recv (char *data, int size);
    int bytes_available ();
    int close ();

private:
    std::string mac_addr;
    int port;
#ifdef _WIN32
    SOCKET socket_bt;
#endif
};
