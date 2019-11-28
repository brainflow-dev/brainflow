#include <string.h>

#include "socket_client.h"

///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


int SocketClient::get_local_ip_addr (char *connect_ip, int port, char *local_ip)
{
    WSADATA wsadata;
    int return_value = (int)SocketReturnCodes::STATUS_OK;
    struct sockaddr_in serv;
    char buffer[80];
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in name;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return_value = (int)SocketReturnCodes::WSA_STARTUP_ERROR;
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        sock = socket (AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET)
        {
            return_value = (int)SocketReturnCodes::CREATE_SOCKET_ERROR;
        }
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        memset (&serv, 0, sizeof (serv));
        serv.sin_family = AF_INET;
        if (inet_pton (AF_INET, connect_ip, &serv.sin_addr) == 0)
        {
            return_value = (int)SocketReturnCodes::PTON_ERROR;
        }
        serv.sin_port = htons (port);
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        if (::connect (sock, (const struct sockaddr *)&serv, sizeof (serv)) == SOCKET_ERROR)
        {
            return_value = (int)SocketReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        int namelen = sizeof (name);
        int err = getsockname (sock, (struct sockaddr *)&name, &namelen);
        if (err != 0)
        {
            return_value = (int)SocketReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        const char *p = inet_ntop (AF_INET, &name.sin_addr, buffer, 80);
        if (p != NULL)
        {
            strcpy (local_ip, buffer);
        }
        else
        {
            return_value = (int)SocketReturnCodes::PTON_ERROR;
        }
    }

    closesocket (sock);
    WSACleanup ();
    return return_value;
}

SocketClient::SocketClient (const char *ip_addr, int port, int socket_type)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
    this->socket_type = socket_type;
}

int SocketClient::connect (int min_bytes)
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
    DWORD timeout = 5000;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    if (socket_type == (int)SocketType::TCP)
    {
        DWORD value = 1;
        setsockopt (connect_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof (value));
        setsockopt (connect_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&value, sizeof (value));
        if (::connect (connect_socket, (sockaddr *)&socket_addr, sizeof (socket_addr)) ==
            SOCKET_ERROR)
        {
            return (int)SocketReturnCodes::CONNECT_ERROR;
        }
        // to simplify parsing code and make it uniform for udp and tcp set min bytes for tcp to
        // package size
        setsockopt (
            connect_socket, SOL_SOCKET, SO_RCVLOWAT, (char *)&min_bytes, sizeof (min_bytes));
    }

    return (int)SocketReturnCodes::STATUS_OK;
}

int SocketClient::send (const char *data, int size)
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

int SocketClient::recv (void *data, int size)
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

void SocketClient::close ()
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

int SocketClient::get_local_ip_addr (char *connect_ip, int port, char *local_ip)
{
    int return_value = (int)SocketReturnCodes::STATUS_OK;
    struct sockaddr_in serv;
    char buffer[80];
    int sock = -1;
    struct sockaddr_in name;

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        sock = socket (AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            return_value = (int)SocketReturnCodes::CREATE_SOCKET_ERROR;
        }
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        memset (&serv, 0, sizeof (serv));
        serv.sin_family = AF_INET;
        if (inet_pton (AF_INET, connect_ip, &serv.sin_addr) == 0)
        {
            return_value = (int)SocketReturnCodes::PTON_ERROR;
        }
        serv.sin_port = htons (port);
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        if (::connect (sock, (const struct sockaddr *)&serv, sizeof (serv)) == -1)
        {
            return_value = (int)SocketReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        socklen_t namelen = (socklen_t)sizeof (name);
        int err = getsockname (sock, (struct sockaddr *)&name, &namelen);
        if (err != 0)
        {
            return_value = (int)SocketReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketReturnCodes::STATUS_OK)
    {
        const char *p = inet_ntop (AF_INET, &name.sin_addr, buffer, 80);
        if (p != NULL)
        {
            strcpy (local_ip, buffer);
        }
        else
        {
            return_value = (int)SocketReturnCodes::PTON_ERROR;
        }
    }

    ::close (sock);
    return return_value;
}

SocketClient::SocketClient (const char *ip_addr, int port, int socket_type)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
    this->socket_type = socket_type;
}

int SocketClient::connect (int min_bytes)
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
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

    if (socket_type == (int)SocketType::TCP)
    {
        int value = 1;
        setsockopt (connect_socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
        setsockopt (connect_socket, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof (value));
        if (::connect (connect_socket, (sockaddr *)&socket_addr, sizeof (socket_addr)) == -1)
        {
            return (int)SocketReturnCodes::CONNECT_ERROR;
        }
        // to simplify parsing code and make it uniform for udp and tcp set min bytes for tcp to
        // package size
        // setsockopt (connect_socket, SOL_SOCKET, SO_RCVLOWAT, &min_bytes, sizeof (min_bytes));
    }

    return (int)SocketReturnCodes::STATUS_OK;
}

int SocketClient::send (const char *data, int size)
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

int SocketClient::recv (void *data, int size)
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

void SocketClient::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
