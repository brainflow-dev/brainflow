#include <string.h>

#include "socket_client_tcp.h"

///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

SocketClientTCP::SocketClientTCP (const char *ip_addr, int port)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
}

int SocketClientTCP::connect ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketClientTCPReturnCodes::WSA_STARTUP_ERROR;
    }
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, ip_addr, &socket_addr.sin_addr) == 0)
    {
        return (int)SocketClientTCPReturnCodes::PTON_ERROR;
    }
    connect_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)SocketClientTCPReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 5000;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));

    if (::connect (connect_socket, (struct sockaddr *)&socket_addr, sizeof (socket_addr)) ==
        SOCKET_ERROR)
    {
        return (int)SocketClientTCPReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketClientTCPReturnCodes::STATUS_OK;
}

int SocketClientTCP::send (const char *data, int size)
{
    int len = sizeof (socket_addr);
    int res = ::send (connect_socket, data, size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int SocketClientTCP::recv (void *data, int size)
{
    int len = sizeof (socket_addr);
    int res = ::recv (connect_socket, (char *)data, size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void SocketClientTCP::close ()
{
    closesocket (connect_socket);
    connect_socket = INVALID_SOCKET;
    WSACleanup ();
}

///////////////////////////////
//////////// UNIX /////////////
///////////////////////////////
#else

#include <netinet/in.h>
#include <netinet/tcp.h>

SocketClientTCP::SocketClientTCP (const char *ip_addr, int port)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
}

int SocketClientTCP::connect ()
{
    connect_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket < 0)
    {
        return (int)SocketClientTCPReturnCodes::CREATE_SOCKET_ERROR;
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, ip_addr, &socket_addr.sin_addr) == 0)
    {
        return (int)SocketClientTCPReturnCodes::PTON_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

    if (::connect (connect_socket, (struct sockaddr *)&socket_addr, sizeof (socket_addr)) < 0)
    {
        return (int)SocketClientTCPReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketClientTCPReturnCodes::STATUS_OK;
}

int SocketClientTCP::send (const char *data, int size)
{
    int res = ::send (connect_socket, data, size, 0);
    return res;
}

int SocketClientTCP::recv (void *data, int size)
{
    int res = ::recv (connect_socket, (char *)data, (size_t)size, 0);
    return res;
}

void SocketClientTCP::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
