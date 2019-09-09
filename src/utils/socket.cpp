#include "socket.h"


///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

Socket::Socket (const char *ip_addr, int port, int socket_type)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
    this->socket_type = socket_type;
}

int Socket::connect ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketReturnCodes::WSA_STARTUP_ERROR;
    }
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, ip_addr, &socket_addr.sin_addr) == 0)
    {
        return (int)SocketReturnCodes::PTON_ERROR;
    }
    if (socket_type == (int)SocketType::UDP)
    {
        connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        connect_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)SocketReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 3000;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    if (socket_type == (int)SocketType::TCP)
    {
        DWORD value = 1;
        setsockopt (connect_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof (value));
        if (::connect (connect_socket, (sockaddr *)&socket_addr, sizeof (socket_addr)) ==
            SOCKET_ERROR)
        {
            return (int)SocketReturnCodes::CONNECT_ERROR;
        }
    }

    return (int)SocketReturnCodes::STATUS_OK;
}

int Socket::send (const char *data, int size)
{
    int len = sizeof (socket_addr);
    int res = 0;
    if (socket_type == (int)SocketType::UDP)
    {
        res = sendto (connect_socket, data, size, 0, (sockaddr *)&socket_addr, len);
    }
    else
    {
        res = ::send (connect_socket, data, size, 0);
    }
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int Socket::recv (void *data, int size)
{
    int len = sizeof (socket_addr);
    int res;
    if (socket_type == (int)SocketType::UDP)
    {
        res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr, &len);
    }
    else
    {
        res = ::recv (connect_socket, (char *)data, size, 0);
    }
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void Socket::close ()
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

Socket::Socket (const char *ip_addr, int port, int socket_type)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
    this->socket_type = socket_type;
}

int Socket::connect ()
{
    if (socket_type == (int)SocketType::UDP)
    {
        connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        connect_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    if (connect_socket < 0)
    {
        return (int)SocketReturnCodes::CREATE_SOCKET_ERROR;
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, ip_addr, &socket_addr.sin_addr) == 0)
    {
        return (int)SocketReturnCodes::PTON_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

    if (socket_type == (int)SocketType::TCP)
    {
        int value = 1;
        setsockopt (connect_socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
        if (::connect (connect_socket, (sockaddr *)&socket_addr, sizeof (socket_addr)) == -1)
        {
            return (int)SocketReturnCodes::CONNECT_ERROR;
        }
    }

    return (int)SocketReturnCodes::STATUS_OK;
}

int Socket::send (const char *data, int size)
{
    int res;
    if (socket_type == (int)SocketType::UDP)
    {
        res =
            sendto (connect_socket, data, size, 0, (sockaddr *)&socket_addr, sizeof (socket_addr));
    }
    else
    {
        res = ::send (connect_socket, data, size, 0);
    }
    return res;
}

int Socket::recv (void *data, int size)
{
    unsigned int len = (unsigned int)sizeof (socket_addr);
    int res;
    if (socket_type == (int)SocketType::UDP)
    {
        res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr, &len);
    }
    else
    {
        res = ::recv (connect_socket, (char *)data, (size_t)size, 0);
    }
    return res;
}

void Socket::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
