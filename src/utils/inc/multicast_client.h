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


enum class MultiCastReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    BIND_ERROR = 3,
    PTON_ERROR = 4,
    SET_OPT_ERROR = 5
};

class MultiCastClient
{

public:
    MultiCastClient (const char *port_name, int port);
    ~MultiCastClient ()
    {
        close ();
    }

    int init ();
    int recv (void *data, int size);
    void close ();


private:
    char ip_addr[32];
    int port;
#ifdef _WIN32
    SOCKET client_socket;
    struct sockaddr_in socket_addr;
#else
    int client_socket;
    struct sockaddr_in socket_addr;
#endif
};
