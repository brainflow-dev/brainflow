#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "ble_lib_board.h"
#include "board.h"
#include "board_controller.h"


class AAVAAv3 : public BLELibBoard
{
public:
    AAVAAv3 (struct BrainFlowInputParams params);
    ~AAVAAv3 ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
    int send_command (std::string config);

    void adapter_1_on_scan_start (simpleble_adapter_t adapter);
    void adapter_1_on_scan_stop (simpleble_adapter_t adapter);
    void adapter_1_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral);
    void read_data (simpleble_uuid_t service, simpleble_uuid_t characteristic, const uint8_t *data,
        size_t size);
    std::deque<uint8_t> Incoming_BLE_Data_Buffer;
    std::string device_status;

protected:
    const double TIMESTAMP_SCALE = (double)(10. / 1000000.); // 10 us
    const double IMU_SCALE = (double)(1. / 100.);
    const int Ring_Buffer_Max_Size = 244 * 5;
    const int SIZE_OF_DATA_FRAME = 47;
    const double ADS1299_Vref = 4.5; // reference voltage for ADC in ADS1299
    const double ADS1299_gain = 12.; // assumed gain setting for ADS1299
    const double EEG_SCALE = ADS1299_Vref / float ((pow (2, 23) - 1)) / ADS1299_gain * 1000000.0;

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
