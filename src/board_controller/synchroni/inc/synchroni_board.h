#include <condition_variable>
#include <mutex>
#include <thread>

#include "ble_lib_board.h"
#include "board.h"
#include "board_controller.h"

class SynchroniBoard : public BLELibBoard
{

public:
    SynchroniBoard (int board_id, struct BrainFlowInputParams params);
    ~SynchroniBoard ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);

    void adapter_1_on_scan_updated (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void read_data (
        simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size);

protected:
    volatile simpleble_adapter_t synchroni_board_adapter;
    volatile simpleble_peripheral_t synchroni_board_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::condition_variable cv;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> write_characteristics;
};
