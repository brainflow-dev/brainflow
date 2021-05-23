#include "multicast_server.h"


///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


MultiCastServer::MultiCastServer (const char *local_ip, int local_port)
{
    strcpy (this->local_ip, local_ip);
    this->local_port = local_port;
    server_socket = INVALID_SOCKET;
    memset (&server_addr, 0, sizeof (server_addr));
}

int MultiCastServer::init ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)MultiCastReturnCodes::WSA_STARTUP_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    if (inet_pton (AF_INET, local_ip, &server_addr.sin_addr) == 0)
    {
        return (int)MultiCastReturnCodes::PTON_ERROR;
    }
    server_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == INVALID_SOCKET)
    {
        return (int)MultiCastReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 5000;
    DWORD value = 1;
    DWORD buf_size = 65000;
    setsockopt (server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&value, sizeof (value));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDBUF, (const char *)&buf_size, sizeof (buf_size));

    return (int)MultiCastReturnCodes::STATUS_OK;
}

int MultiCastServer::send (void *data, int size)
{
    int res = ::sendto (server_socket, (char *)data, size, 0, (struct sockaddr *)&server_addr,
        sizeof (server_addr));
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void MultiCastServer::close ()
{
    if (server_socket != INVALID_SOCKET)
    {
        closesocket (server_socket);
        server_socket = INVALID_SOCKET;
    }
    WSACleanup ();
}

///////////////////////////////
//////////// UNIX /////////////
///////////////////////////////
#else

#include <netinet/in.h>
#include <netinet/tcp.h>

MultiCastServer::MultiCastServer (const char *local_ip, int local_port)
{
    strcpy (this->local_ip, local_ip);
    this->local_port = local_port;
    server_socket = -1;
    memset (&server_addr, 0, sizeof (server_addr));
}

int MultiCastServer::init ()
{
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    if (inet_pton (AF_INET, local_ip, &server_addr.sin_addr) == 0)
    {
        return (int)MultiCastReturnCodes::PTON_ERROR;
    }
    server_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == -1)
    {
        return (int)MultiCastReturnCodes::CREATE_SOCKET_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int value = 1;
    int buf_size = 65000;
    setsockopt (server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&value, sizeof (value));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof (buf_size));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof (buf_size));

    return (int)MultiCastReturnCodes::STATUS_OK;
}

int MultiCastServer::send (void *data, int size)
{
    socklen_t len = (socklen_t)sizeof (server_addr);
    int res = ::sendto (server_socket, (char *)data, size, 0, (struct sockaddr *)&server_addr, len);
    return res;
}

void MultiCastServer::close ()
{
    if (server_socket != -1)
    {
        ::close (server_socket);
        server_socket = -1;
    }
}

#endif
