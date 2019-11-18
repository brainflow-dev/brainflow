#pragma once

#include <thread>

#include "socket_client.h"


class SocketServer
{

public:
    SocketServer (const char *local_ip, int local_port);
    ~SocketServer ()
    {
        close ();
    }

    int bind (int min_bytes = 1);
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

    std::thread accept_thread;

#ifdef _WIN32
    volatile SOCKET server_socket;
    volatile SOCKET connected_socket;
#else
    volatile int server_socket;
    volatile int connected_socket;
#endif
};
