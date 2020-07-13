#pragma once

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>


enum class BroadCastClientReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    INIT_ERROR = 3
};


class BroadCastClient
{

public:
    BroadCastClient (int port);
    ~BroadCastClient ()
    {
        close ();
    }

    int init ();
    int recv (void *data, int size);
    void close ();

    int get_port ()
    {
        return port;
    }

private:
    int port;
#ifdef _WIN32
    SOCKET connect_socket;
    struct sockaddr_in socket_addr;
#else
    int connect_socket;
    struct sockaddr_in socket_addr;
#endif
};
