#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <ws2bth.h>
// clang-format on
#endif

#ifdef __APPLE__
#include "pipe.h"
#endif

#include <queue>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <utility>


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
    int bytes_available ()
    {
        return (int)temp_buffer.size ();
    }
    int close ();

    static std::pair<std::string, int> discover (char *device_selector);

private:
    std::string mac_addr;
    int port;
    std::queue<char> temp_buffer;
#ifdef _WIN32
    SOCKET socket_bt;
#elif defined(__linux__)
    int socket_bt;
    int rep[2];
#elif defined(__APPLE__)
    pipe_consumer_t *consumer;
#endif
};
