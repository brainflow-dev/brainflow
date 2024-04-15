#pragma once

#include <condition_variable>
#include <mutex>
#include <stdint.h>
#include <thread>

#include "bt_lib_board.h"


#pragma pack(push, 1)
struct ExploreHeader
{
    unsigned char pid;
    unsigned char counter;
    uint16_t payload_size;
    uint32_t timestamp;

    ExploreHeader ()
    {
        pid = 0;
        payload_size = 0;
        counter = 0;
        timestamp = 0;
    }
};
#pragma pack(pop)

class Explore : public BTLibBoard
{

protected:
    volatile bool keep_alive;
    volatile int state;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;
    double last_eeg_timestamp;

    void read_thread ();
    std::string get_name_selector ();
    void parse_eeg_data (const ExploreHeader *header, double *package, unsigned char *payload,
        double vref, int n_packages);
    void parse_orientation_data (
        const ExploreHeader *header, double *package, unsigned char *payload);
    void parse_env_data (const ExploreHeader *header, double *package, unsigned char *payload);
    double get_battery_percentage (double battery);

public:
    Explore (int board_id, struct BrainFlowInputParams params);
    ~Explore ();

    int prepare_session ();
    int config_board (std::string config, std::string &response);
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
};
