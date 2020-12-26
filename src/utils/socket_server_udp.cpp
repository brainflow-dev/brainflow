#include "socket_server_udp.h"


///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

SocketServerUDP::SocketServerUDP (int local_port)
{
    this->local_port = local_port;
    server_socket = INVALID_SOCKET;
    memset (&server_addr, 0, sizeof (server_addr));
}

int SocketServerUDP::bind ()
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketServerUDPReturnCodes::WSA_STARTUP_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == INVALID_SOCKET)
    {
        return (int)SocketServerUDPReturnCodes::CREATE_SOCKET_ERROR;
    }

    if (::bind (server_socket, (const struct sockaddr *)&server_addr, sizeof (server_addr)) != 0)
    {
        return (int)SocketServerUDPReturnCodes::BIND_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 3000;
    DWORD value = 1;
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));

    return (int)SocketServerUDPReturnCodes::STATUS_OK;
}

int SocketServerUDP::recv (void *data, int size)
{
    struct sockaddr_in client_addr;
    memset (&client_addr, 0, sizeof (client_addr));
    socklen_t len = sizeof (client_addr);
    int res =
        recvfrom (server_socket, (char *)data, size, 0, (struct sockaddr *)&client_addr, &len);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void SocketServerUDP::close ()
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


SocketServerUDP::SocketServerUDP (int local_port)
{
    this->local_port = local_port;
    server_socket = -1;
    memset (&server_addr, 0, sizeof (server_addr));
}

int SocketServerUDP::bind ()
{
    server_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0)
    {
        return (int)SocketServerUDPReturnCodes::CREATE_SOCKET_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);

    if (::bind (server_socket, (const struct sockaddr *)&server_addr, sizeof (server_addr)) != 0)
    {
        return (int)SocketServerUDPReturnCodes::BIND_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

    return (int)SocketServerUDPReturnCodes::STATUS_OK;
}

int SocketServerUDP::recv (void *data, int size)
{
    struct sockaddr_in client_addr;
    memset (&client_addr, 0, sizeof (client_addr));
    socklen_t len = (socklen_t)sizeof (client_addr);
    return recvfrom (server_socket, (char *)data, size, 0, (struct sockaddr *)&client_addr, &len);
}

void SocketServerUDP::close ()
{
    if (server_socket != -1)
    {
        ::close (server_socket);
        server_socket = -1;
    }
}
#endif
