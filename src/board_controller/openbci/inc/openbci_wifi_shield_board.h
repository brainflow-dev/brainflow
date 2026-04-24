#pragma once

#include <string>
#include <thread>

#include "board.h"
#include "board_controller.h"
#include "socket_server_tcp.h"


struct http_t; // forward declaration cause include "http.h" here leads to strange errors

class OpenBCIWifiShieldBoard : public Board
{

protected:
    int http_timeout;
    int wait_for_http_resp (http_t *request);

    volatile bool keep_alive;
    bool initialized;
    int current_sampling_rate;
    std::thread streaming_thread;

    SocketServerTCP *server_socket;

    std::string find_wifi_shield ();

    virtual void read_thread () = 0;
    virtual int send_config (const char *config);

public:
    OpenBCIWifiShieldBoard (struct BrainFlowInputParams params, int board_id);
    ~OpenBCIWifiShieldBoard () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
    int get_board_sampling_rate (int preset) override;

    static constexpr int package_size = 33;
};
