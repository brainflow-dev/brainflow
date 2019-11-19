#include "socket_server.h"


///////////////////////////////
/////////// WINDOWS ///////////
//////////////////////////////
#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

SocketServer::SocketServer (const char *local_ip, int local_port)
{
    strcpy (this->local_ip, local_ip);
    this->local_port = local_port;
    server_socket = INVALID_SOCKET;
    connected_socket = INVALID_SOCKET;
    client_connected = false;
}

int SocketServer::bind (int min_bytes)
{
    WSADATA wsadata;
    int res = WSAStartup (MAKEWORD (2, 2), &wsadata);
    if (res != 0)
    {
        return (int)SocketReturnCodes::WSA_STARTUP_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    if (inet_pton (AF_INET, local_ip, &server_addr.sin_addr) == 0)
    {
        return (int)SocketReturnCodes::PTON_ERROR;
    }
    server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET)
    {
        return (int)SocketReturnCodes::CREATE_SOCKET_ERROR;
    }

    if (::bind (server_socket, (const struct sockaddr *)&server_addr, sizeof (server_addr)) != 0)
    {
        return (int)SocketReturnCodes::CONNECT_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    DWORD timeout = 3000;
    DWORD value = 1;
    setsockopt (server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof (value));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));
    // set linger timeout to 1s to handle time_wait state gracefully
    struct linger sl;
    sl.l_onoff = 1;
    sl.l_linger = 1;
    setsockopt (server_socket, SOL_SOCKET, SO_LINGER, (char *)&sl, sizeof (sl));
    // to simplify parsing code and make it uniform for udp and tcp set min bytes for tcp to
    // package size
    setsockopt (server_socket, SOL_SOCKET, SO_RCVLOWAT, (char *)&min_bytes, sizeof (min_bytes));

    if ((listen (server_socket, 1)) != 0)
    {
        return (int)SocketReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketReturnCodes::STATUS_OK;
}

int SocketServer::accept ()
{
    accept_thread = std::thread ([this] { this->accept_worker (); });
    return (int)SocketReturnCodes::STATUS_OK;
}

void SocketServer::accept_worker ()
{
    int len = sizeof (client_addr);
    connected_socket = ::accept (server_socket, (struct sockaddr *)&client_addr, &len);
    if (connected_socket != INVALID_SOCKET)
    {
        // ensure that library will not hang in blocking recv/send call
        DWORD timeout = 3000;
        DWORD value = 1;
        setsockopt (connected_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof (value));
        setsockopt (connected_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
        setsockopt (connected_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof (timeout));

        client_connected = true;
    }
}

int SocketServer::recv (void *data, int size)
{
    if (connected_socket == INVALID_SOCKET)
    {
        return -1;
    }
    int res = ::recv (connected_socket, (char *)data, size, 0);
    if (res == SOCKET_ERROR)
    {
        return -1;
    }
    return res;
}

void SocketServer::close ()
{
    if (server_socket != INVALID_SOCKET)
    {
        closesocket (server_socket);
        server_socket = INVALID_SOCKET;
    }
    if (accept_thread.joinable ())
    {
        accept_thread.join ();
    }
    if (connected_socket != INVALID_SOCKET)
    {
        closesocket (connected_socket);
        connected_socket = INVALID_SOCKET;
    }
    WSACleanup ();
}

///////////////////////////////
//////////// UNIX /////////////
///////////////////////////////
#else

#include <netinet/in.h>
#include <netinet/tcp.h>


SocketServer::SocketServer (const char *local_ip, int local_port)
{
    strcpy (this->local_ip, local_ip);
    this->local_port = local_port;
    server_socket = -1;
    connected_socket = -1;
    client_connected = false;
}

int SocketServer::bind (int min_bytes)
{
    server_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        return (int)SocketReturnCodes::CREATE_SOCKET_ERROR;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (local_port);
    if (inet_pton (AF_INET, local_ip, &server_addr.sin_addr) == 0)
    {
        return (int)SocketReturnCodes::PTON_ERROR;
    }

    if (::bind (server_socket, (const struct sockaddr *)&server_addr, sizeof (server_addr)) != 0)
    {
        return (int)SocketReturnCodes::CONNECT_ERROR;
    }

    // ensure that library will not hang in blocking recv/send call
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    int value = 1;
    setsockopt (server_socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
    setsockopt (server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
    setsockopt (server_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));
    // set linger timeout to 1s to handle time_wait state gracefully
    struct linger sl;
    sl.l_onoff = 1;
    sl.l_linger = 1;
    setsockopt (server_socket, SOL_SOCKET, SO_LINGER, &sl, sizeof (sl));
    // to simplify parsing code and make it uniform for udp and tcp set min bytes for tcp to
    // package size
    setsockopt (server_socket, SOL_SOCKET, SO_RCVLOWAT, &min_bytes, sizeof (min_bytes));

    if ((listen (server_socket, 1)) != 0)
    {
        return (int)SocketReturnCodes::CONNECT_ERROR;
    }

    return (int)SocketReturnCodes::STATUS_OK;
}

int SocketServer::accept ()
{
    accept_thread = std::thread ([this] { this->accept_worker (); });
    return (int)SocketReturnCodes::STATUS_OK;
}

void SocketServer::accept_worker ()
{
    unsigned int len = sizeof (client_addr);
    connected_socket = ::accept (server_socket, (struct sockaddr *)&this->client_addr, &len);
    if (connected_socket > 0)
    {
        // ensure that library will not hang in blocking recv/send call
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        int value = 1;
        setsockopt (connected_socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
        setsockopt (connected_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof (tv));
        setsockopt (connected_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof (tv));

        client_connected = true;
    }
}

int SocketServer::recv (void *data, int size)
{
    if (connected_socket <= 0)
    {
        return -1;
    }
    int res = ::recv (connected_socket, (char *)data, size, 0);
    return res;
}

void SocketServer::close ()
{
    if (server_socket != -1)
    {
        ::close (server_socket);
        server_socket = -1;
    }
    if (accept_thread.joinable ())
    {
        accept_thread.join ();
    }
    if (connected_socket != -1)
    {
        ::close (connected_socket);
        connected_socket = -1;
    }
}
#endif
