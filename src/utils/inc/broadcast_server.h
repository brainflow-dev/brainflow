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


enum class BroadCastServerReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    INIT_ERROR = 3,
    SOCKET_ALREADY_CREATED_ERROR = 4
};


class BroadCastServer
{

public:
    BroadCastServer (int port);
    BroadCastServer (const char *address, int port);
    ~BroadCastServer ()
    {
        close ();
    }

    int init ();
    int send (const char *data, int size);
    int recv (void *data, int size, char *sender_ip, int max_len);
    void close ();

private:
    int port;
    char address[64];
#ifdef _WIN32
    SOCKET connect_socket;
    struct sockaddr_in socket_addr;
    bool wsa_initialized;
#else
    int connect_socket;
    struct sockaddr_in socket_addr;
#endif
};
