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


enum class SocketType
{
    UDP = 0,
    TCP = 1
};

enum class SocketReturnCodes
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    CONNECT_ERROR = 3,
    PTON_ERROR = 4
};

class SocketClient
{

public:
    static int get_local_ip_addr (char *connect_ip, int port, char *local_ip);

    SocketClient (const char *port_name, int port, int socket_type);
    ~SocketClient ()
    {
        close ();
    }

    int connect (int min_bytes = 1); // makes sense only for tcp
    int send (const char *data, int size);
    int recv (void *data, int size);
    void close ();
    int get_local_ip_addr (char *local_ip);
    char *get_ip_addr ()
    {
        return ip_addr;
    }
    int get_port ()
    {
        return port;
    }


private:
    char ip_addr[32];
    int port;
    int socket_type;
#ifdef _WIN32
    SOCKET connect_socket;
    struct sockaddr_in socket_addr;
#else
    int connect_socket;
    struct sockaddr_in socket_addr;
#endif
};
