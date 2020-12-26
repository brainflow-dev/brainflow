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


enum class SocketClientUDPReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    CONNECT_ERROR = 3,
    PTON_ERROR = 4,
    INVALID_ARGUMENT_ERROR = 5
};


class SocketClientUDP
{

public:
    static int get_local_ip_addr (const char *connect_ip, int port, char *local_ip);

    SocketClientUDP (const char *ip_addr, int port);
    ~SocketClientUDP ()
    {
        close ();
    }

    int connect ();
    int bind ();
    int set_timeout (int num_seconds);
    int send (const char *data, int size);
    int recv (void *data, int size);
    void close ();
    int get_local_ip_addr (const char *local_ip);
    char *get_ip_addr ()
    {
        return ip_addr;
    }
    int get_port ()
    {
        return port;
    }
    int get_local_port ();

private:
    char ip_addr[32];
    int port;
#ifdef _WIN32
    SOCKET connect_socket;
    struct sockaddr_in socket_addr;
#else
    int connect_socket;
    struct sockaddr_in socket_addr;
#endif
};
