#include <condition_variable>
#include <mutex>
#include <thread>

#include "ble_lib_board.h"
#include "board.h"
#include "board_controller.h"

class BrainAlive : public BLELibBoard
{
public:
    BrainAlive (struct BrainFlowInputParams params);
    ~BrainAlive ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int config_board (std::string config);

    void adapter_1_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void read_data (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t size, int channel_num);

    // common constants
    static constexpr int brainalive_packet_size = 220;
    static constexpr int brainalive_single_packet_size = 44;
    static constexpr int num_of_packets = brainalive_packet_size / brainalive_single_packet_size;

    static constexpr int brainalive_packet_index = (brainalive_single_packet_size - 3);

    static constexpr int brainalive_eeg_data_szie = 24;
    static constexpr int brainalive_eeg_Start_index = 4;
    static constexpr int brainalive_eeg_end_index =
        (brainalive_eeg_Start_index + brainalive_eeg_data_szie);

    static constexpr int brainalive_axl_gyro_data_size = 12;
    static constexpr int brainalive_axl_start_index = brainalive_eeg_end_index;
    static constexpr int brainalive_axl_end_index =
        brainalive_axl_start_index + brainalive_axl_gyro_data_size;
    static constexpr int FSR_Value = 8388607;
    static constexpr int ba_brainflow_package_size = 17;

protected:
    volatile simpleble_adapter_t brainalive_adapter;
    volatile simpleble_peripheral_t brainalive_peripheral;
    bool initialized;
    bool is_streaming;
    std::mutex m;
    std::condition_variable cv;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> write_characteristics;
};
