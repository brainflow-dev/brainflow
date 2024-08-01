#pragma once

#include <stdint.h>
#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"

#include "broadcast_server.h"
#include "socket_client_udp.h"
#include "socket_server_tcp.h"

class Synchroni_Trio : public Board
{

private:
    volatile bool keep_alive;
    volatile bool initialized;
    
    std::string ip_address;
    std::thread streaming_thread;
    std::thread connection_thread;
    SocketClientUDP *data_socket;
    SocketServerTCP *control_socket;
    BroadCastServer *advertise_socket_server;
    std::mutex m;
    std::condition_variable cv;
    int control_port;
    int data_port;
    
    void read_thread ();
    void ping_thread ();

public:
    Synchroni_Trio (struct BrainFlowInputParams params);
    ~Synchroni_Trio ();
};