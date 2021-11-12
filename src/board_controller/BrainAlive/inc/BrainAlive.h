#include <condition_variable>
#include <mutex>
#include <thread>

#include "board.h"
#include "ble_lib_board.h"
#include "board_controller.h"


 class BrainAlive_Device : public BLELibBoard
{

public:
    BrainAlive_Device (struct BrainFlowInputParams params);
    ~BrainAlive_Device ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);

    void adapter_1_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void Read_Data (simpleble_uuid_t service, simpleble_uuid_t characteristic,
        uint8_t *data, size_t size, int channel_num);

protected:
    volatile simpleble_adapter_t brainalive_adapter;
    volatile simpleble_peripheral_t brainalive_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::condition_variable cv;
    std::vector<std::pair<simpleble_uuid_t, simpleble_uuid_t>> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> control_characteristics;
    std::vector<std::vector<double>> current_buf;
    std::vector<bool> new_eeg_data;
    double last_timestamp;
 };
