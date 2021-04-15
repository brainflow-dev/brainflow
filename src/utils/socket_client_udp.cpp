#include <string.h>

#include "socket_client_udp.h"

///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


int SocketClientUDP::get_local_ip_addr (const char *connect_ip, int port, char *local_ip)
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

    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::set_timeout (int num_seconds)
{
    if ((num_seconds < 1) || (num_seconds > 100))
    {
        return (int)SocketClientUDPReturnCodes::INVALID_ARGUMENT_ERROR;
    }
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }

    DWORD timeout = 1000 * num_seconds;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));

    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::bind ()
{
    // for socket clients which dont call sendto before recvfrom bind should be called on client
    // side
    // https://stackoverflow.com/questions/3057029/do-i-have-to-bind-a-udp-socket-in-my-client-program-to-receive-data-i-always-g
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }
    if (::bind (connect_socket, (const struct sockaddr *)&socket_addr, sizeof (socket_addr)) != 0)
    {
        return (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
    }
    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::get_local_port ()
{
    if (connect_socket == INVALID_SOCKET)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }
    struct sockaddr_in sin_local;
    memset (&sin_local, 0, sizeof (sin_local));
    socklen_t slen = (socklen_t)sizeof (sin_local);
    if (getsockname (connect_socket, (struct sockaddr *)&sin_local, &slen) == 0)
    {
        return ntohs (sin_local.sin_port);
    }
    else
    {
        return -1;
    }
}

int SocketClientUDP::send (const char *data, int size)
{
    int res = sendto (connect_socket, data, size, 0, (const struct sockaddr *)&socket_addr,
        (int)sizeof (socket_addr));
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

int SocketClientUDP::recv (void *data, int size)
{
    int res = recvfrom (connect_socket, (char *)data, size, 0, NULL, 0);
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

int SocketClientUDP::get_local_ip_addr (const char *connect_ip, int port, char *local_ip)
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

    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::set_timeout (int num_seconds)
{
    if ((num_seconds < 1) || (num_seconds > 100))
    {
        return (int)SocketClientUDPReturnCodes::INVALID_ARGUMENT_ERROR;
    }
    if (connect_socket < 0)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }

    struct timeval tv;
    tv.tv_sec = num_seconds;
    tv.tv_usec = 0;
    setsockopt (connect_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (connect_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::bind ()
{
    // for socket clients which dont call sendto before recvfrom bind should be called on client
    // side
    // https://stackoverflow.com/questions/3057029/do-i-have-to-bind-a-udp-socket-in-my-client-program-to-receive-data-i-always-g
    if (connect_socket < 0)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }
    if (::bind (connect_socket, (const struct sockaddr *)&socket_addr, sizeof (socket_addr)) != 0)
    {
        return (int)SocketClientUDPReturnCodes::CONNECT_ERROR;
    }
    return (int)SocketClientUDPReturnCodes::STATUS_OK;
}

int SocketClientUDP::get_local_port ()
{
    if (connect_socket < 0)
    {
        return (int)SocketClientUDPReturnCodes::CREATE_SOCKET_ERROR;
    }
    struct sockaddr_in sin_local;
    memset (&sin_local, 0, sizeof (sin_local));
    socklen_t slen = (socklen_t)sizeof (sin_local);
    if (getsockname (connect_socket, (struct sockaddr *)&sin_local, &slen) == 0)
    {
        return ntohs (sin_local.sin_port);
    }
    else
    {
        return -1;
    }
}

int SocketClientUDP::send (const char *data, int size)
{
    int res = sendto (connect_socket, data, size, 0, (const struct sockaddr *)&socket_addr,
        (socklen_t)sizeof (socket_addr));
    return res;
}

int SocketClientUDP::recv (void *data, int size)
{
    int res = recvfrom (connect_socket, (char *)data, size, 0, NULL, 0);
    return res;
}

void SocketClientUDP::close ()
{
    ::close (connect_socket);
    connect_socket = -1;
}
#endif
