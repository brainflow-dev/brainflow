#include <condition_variable>
#include <mutex>
#include <thread>

#include "ble_lib_board.h"
#include "board.h"
#include "board_controller.h"

struct GanglionTempData
{
    float last_data[8];

    double accel_x;
    double accel_y;
    double accel_z;

    double resist_ref;
    double resist_first;
    double resist_second;
    double resist_third;
    double resist_fourth;

    GanglionTempData ()
    {
        memset (last_data, 0, sizeof (float) * 8);
        accel_x = 0.0;
        accel_y = 0.0;
        accel_z = 0.0;
        resist_ref = 0.0;
        resist_first = 0.0;
        resist_second = 0.0;
        resist_third = 0.0;
        resist_fourth = 0.0;
    }

    GanglionTempData (const GanglionTempData &other)
    {
        memcpy (last_data, other.last_data, sizeof (float) * 8);
        accel_x = other.accel_x;
        accel_y = other.accel_y;
        accel_z = other.accel_z;
        resist_ref = other.resist_ref;
        resist_first = other.resist_first;
        resist_second = other.resist_second;
        resist_third = other.resist_third;
        resist_fourth = other.resist_fourth;
    }

    GanglionTempData &operator= (const GanglionTempData &other)
    {
        if (this == &other)
            return *this;

        memcpy (last_data, other.last_data, sizeof (float) * 8);
        accel_x = other.accel_x;
        accel_y = other.accel_y;
        accel_z = other.accel_z;
        resist_ref = other.resist_ref;
        resist_first = other.resist_first;
        resist_second = other.resist_second;
        resist_third = other.resist_third;
        resist_fourth = other.resist_fourth;

        return *this;
    }

    void reset ()
    {
        memset (last_data, 0, sizeof (float) * 8);
        accel_x = 0.0;
        accel_y = 0.0;
        accel_z = 0.0;
        resist_ref = 0.0;
        resist_first = 0.0;
        resist_second = 0.0;
        resist_third = 0.0;
        resist_fourth = 0.0;
    }
};

class GanglionNative : public BLELibBoard
{
public:
    GanglionNative (struct BrainFlowInputParams params);
    ~GanglionNative ();

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
    void read_data (simpleble_uuid_t service, simpleble_uuid_t characteristic, const uint8_t *data,
        size_t size);

protected:
    volatile simpleble_adapter_t ganglion_adapter;
    volatile simpleble_peripheral_t ganglion_peripheral;
    bool initialized;
    bool is_streaming;
    uint8_t firmware;
    std::mutex m;
    std::condition_variable cv;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> notified_characteristics;
    std::pair<simpleble_uuid_t, simpleble_uuid_t> write_characteristics;
    std::string start_command;
    std::string stop_command;
    struct GanglionTempData temp_data;

    double const accel_scale = 0.016f;
    double const eeg_scale = (1.2f * 1000000) / (8388607.0f * 1.5f * 51.0f);

    void decompress_firmware_3 (const uint8_t *data, double *package);
    void decompress_firmware_2 (const uint8_t *data, double *package);
};
