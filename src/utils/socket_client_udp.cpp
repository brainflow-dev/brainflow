#include <string.h>

#include "socket_client_udp.h"

///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


int SocketClientUDP::get_local_ip_addr (char *connect_ip, int port, char *local_ip)
{
    WSADATA wsadata;
    int return_value = (int)SocketClientUDPReturnCodes::STATUS_OK;
    struct sockaddr_in serv;
    char buffer[80];
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in name;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return_value = (int)SocketClientUDPReturnCodes::WSA_STARTUP_ERROR;
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        sock = socket (AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET)
        {
            return_value = (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
        }
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        memset (&serv, 0, sizeof (serv));
        serv.sin_family = AF_INET;
        if (inet_pton (AF_INET, connect_ip, &serv.sin_addr) == 0)
        {
            return_value = (int)SocketClientUDPReturnCodes::PTON_ERROR;
        }
        serv.sin_port = htons (port);
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        if (::connect (sock, (const struct sockaddr *)&serv, sizeof (serv)) == SOCKET_ERROR)
        {
            return_value = (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        int name_len = sizeof (name);
        int err = getsockname (sock, (struct sockaddr *)&name, &name_len);
        if (err != 0)
        {
            return_value = (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        const char *p = inet_ntop (AF_INET, &name.sin_addr, buffer, 80);
        if (p != NULL)
        {
            strcpy (local_ip, buffer);
        }
        else
        {
            return_value = (int)SocketClientUDPReturnCodes::PTON_ERROR;
        }
    }

    closesocket (sock);
    WSACleanup ();
    return return_value;
}

SocketClientUDP::SocketClientUDP (const char *ip_addr, int port)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = INVALID_SOCKET;
    memset (&socket_addr, 0, sizeof (socket_addr));
}

int SocketClientUDP::connect ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketClientUDPReturnCodes::WSA_STARTUP_ERROR;
    }
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, ip_addr, &socket_addr.sin_addr) == 0)
    {
        return (int)SocketClientUDPReturnCodes::PTON_ERROR;
    }
    connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 5000;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));

    // for UDP need to bind in client
    if (::bind (connect_socket, (const struct sockaddr *)&socket_addr, sizeof (socket_addr)) != 0)
    {
        return (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::send (const char *data, int size)
{
    int len = sizeof (socket_addr);
    int res = sendto (connect_socket, data, size, 0, (sockaddr *)&socket_addr, len);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int SocketClientUDP::recv (void *data, int size)
{
    int len = sizeof (socket_addr);
    int res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr, &len);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void SocketClientUDP::close ()
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

int SocketClientUDP::get_local_ip_addr (char *connect_ip, int port, char *local_ip)
{
    int return_value = (int)SocketClientUDPReturnCodes::STATUS_OK;
    struct sockaddr_in serv;
    char buffer[80];
    int sock = -1;
    struct sockaddr_in name;

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        sock = socket (AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            return_value = (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
        }
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        memset (&serv, 0, sizeof (serv));
        serv.sin_family = AF_INET;
        if (inet_pton (AF_INET, connect_ip, &serv.sin_addr) == 0)
        {
            return_value = (int)SocketClientUDPReturnCodes::PTON_ERROR;
        }
        serv.sin_port = htons (port);
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        if (::connect (sock, (const struct sockaddr *)&serv, sizeof (serv)) == -1)
        {
            return_value = (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        socklen_t namelen = (socklen_t)sizeof (name);
        int err = getsockname (sock, (struct sockaddr *)&name, &namelen);
        if (err != 0)
        {
            return_value = (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
        }
    }

    if (return_value == (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        const char *p = inet_ntop (AF_INET, &name.sin_addr, buffer, 80);
        if (p != NULL)
        {
            strcpy (local_ip, buffer);
        }
        else
        {
            return_value = (int)SocketClientUDPReturnCodes::PTON_ERROR;
        }
    }

    ::close (sock);
    return return_value;
}

SocketClientUDP::SocketClientUDP (const char *ip_addr, int port)
{
    strcpy (this->ip_addr, ip_addr);
    this->port = port;
    connect_socket = -1;
    memset (&socket_addr, 0, sizeof (socket_addr));
}

int SocketClientUDP::connect ()
{
    connect_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket < 0)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons (port);
    if (inet_pton (AF_INET, ip_addr, &socket_addr.sin_addr) == 0)
    {
        return (int)SocketClientUDPReturnCodes::PTON_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

    // for UDP need to bind in client
    if (::bind (connect_socket, (const struct sockaddr *)&socket_addr, sizeof (socket_addr)) != 0)
    {
        return (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::send (const char *data, int size)
{
    int res =
        sendto (connect_socket, data, size, 0, (sockaddr *)&socket_addr, sizeof (socket_addr));
    return res;
}

int SocketClientUDP::recv (void *data, int size)
{
    unsigned int len = (unsigned int)sizeof (socket_addr);
    int res = recvfrom (connect_socket, (char *)data, size, 0, (sockaddr *)&socket_addr, &len);
    return res;
}

void SocketClientUDP::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
