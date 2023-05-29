#pragma once

#include <thread>

#include "data_buffer.h"
#include "socket_client_udp.h"
#include "streamer.h"

#include "json.hpp"

using json = nlohmann::json;


class PlotJugglerUDPStreamer : public Streamer
{

public:
    PlotJugglerUDPStreamer (const char *ip, int port, json preset_descr);
    ~PlotJugglerUDPStreamer ();

    int init_streamer ();
    void stream_data (double *data);

private:
    char ip[128];
    int port;
    SocketClientUDP *socket;
    DataBuffer *db;
    volatile bool is_streaming;
    std::thread streaming_thread;
    json preset_descr;

    void thread_worker ();
    std::string remove_substr (std::string str, std::string substr);
};
