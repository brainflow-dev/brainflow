#pragma once

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <queue>
#include <stdlib.h>
#include <string.h>
#include <thread>

enum class SocketServerTCPReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    CONNECT_ERROR = 3,
    PTON_ERROR = 4
};


class SocketServerTCP
{

public:
    SocketServerTCP (const char *local_ip, int local_port, bool recv_all_or_nothing);
    ~SocketServerTCP ()
    {
        close ();
    }

    int bind ();
    int accept ();
    int recv (void *data, int size);
    void close ();
    void accept_worker ();

    volatile bool client_connected; // idea - stop accept blocking call by calling close in
                                    // another thread

private:
    char local_ip[80];
    int local_port;
    struct sockaddr_in server_addr;
    volatile struct sockaddr_in client_addr;
    std::queue<char> temp_buffer;
    bool recv_all_or_nothing;

    std::thread accept_thread;

#ifdef _WIN32
    volatile SOCKET server_socket;
    volatile SOCKET connected_socket;
#else
    volatile int server_socket;
    volatile int connected_socket;
#endif
};
