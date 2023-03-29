#include <condition_variable>
#include <mutex>
#include <thread>
#include <deque>
#include <iostream>
#include <vector>

#include "ble_lib_board.h"
#include "board.h"
#include "board_controller.h"
#include "openbci_gain_tracker.h"


class AAVAA3c2 : public BLELibBoard
{
public:
    AAVAA3c2 (struct BrainFlowInputParams params);
    ~AAVAA3c2 ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);
    int send_command (std::string config);

    void adapter_1_on_scan_start (simpleble_adapter_t adapter);
    void adapter_1_on_scan_stop (simpleble_adapter_t adapter);
    void adapter_1_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void read_data (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);
    std::deque<uint8_t> Incoming_BLE_Data_Buffer;

protected:
    const int Ring_Buffer_Max_Size = 244 * 5;
    const int SIZE_OF_DATA_FRAME = 33;

    CytonGainTracker gain_tracker;

    volatile simpleble_adapter_t aavaa_adapter;
    volatile simpleble_peripheral_t aavaa_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::condition_variable cv;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> write_characteristics;
    std::string start_command;
    std::string stop_command;
};
