#include <string.h>
#include <vector>

#include "explore.h"

#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"


#pragma pack(push, 1)
struct ExploreHeader
{
    unsigned char pid;
    uint16_t payload_size;
    unsigned char counter;
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


Explore::Explore (int board_id, struct BrainFlowInputParams params) : BTLibBoard (board_id, params)
{
    keep_alive = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

Explore::~Explore ()
{
    skip_logs = true;
    release_session ();
}

int Explore::start_stream (int buffer_size, char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    res = bluetooth_open_device (5);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for the 1st package received
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    int num_secs = 5;
    if (cv.wait_for (lk, num_secs * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        return state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in {} sec, stopping thread", num_secs);
        stop_stream ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
}

int Explore::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return bluetooth_close_device ();
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Explore::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
    }
    return BTLibBoard::release_session ();
}

void Explore::read_thread ()
{
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    constexpr int max_payload = 4096;
    char payload_buffer[max_payload];
    for (int i = 0; i < num_rows; i++)
    {
        payload_buffer[i] = 0;
    }

    while (keep_alive)
    {
        struct ExploreHeader header;
        int res = bluetooth_get_data ((char *)&header, sizeof (header));
        if (res != 8)
        {
            continue;
        }
        res = 0;
        double timestamp =
            get_timestamp (); // dont take timestamp from device into account for now, todo
        while ((res != header.payload_size) && (keep_alive))
        {
            res = bluetooth_get_data (payload_buffer, header.payload_size);
        }
        if (!keep_alive)
        {
            break;
        }
        switch (header.pid)
        {
            case 0x0d:
                parse_orientation_data (package, payload_buffer, header.payload_size);
                break;
            case 0x90: // eeg94
                parse_eeg_data (package, payload_buffer, header.payload_size, 2.4, 33);
                break;
            case 0xD0: // eeg94r
                parse_eeg_data (package, payload_buffer, header.payload_size, 2.4, 33);
                break;
            case 0x92: // eeg98
                parse_eeg_data (package, payload_buffer, header.payload_size, 2.4, 16);
                break;
            case 0xD2: // eeg98r
                parse_eeg_data (package, payload_buffer, header.payload_size, 2.4, 16);
                break;
            case 0x3e: // eeg99
                parse_eeg_data (package, payload_buffer, header.payload_size, 4.5, 16);
                break;
            case 0x1e: // eeg99s
                parse_eeg_data (package, payload_buffer, header.payload_size, 4.5, 16);
                break;
            default:
                break;
        }
    }
    delete[] package;
}

std::string Explore::get_name_selector ()
{
    return "Explore";
}

void Explore::parse_orientation_data (double *package, char *payload, int payload_size)
{
    if ((payload[payload_size - 4] != 0xAF) || (payload[payload_size - 3] != 0xBE) ||
        (payload[payload_size - 2] != 0xAD) || (payload[payload_size - 1] != 0xDE))
    {
        safe_logger (spdlog::level::warn, "checksum failed, {} {} {} {}", payload[payload_size - 4],
            payload[payload_size - 3], payload[payload_size - 2], payload[payload_size - 1]);
        return;
    }
    payload_size = payload_size - 4;
    if (payload_size % 2 != 0)
    {
        safe_logger (
            spdlog::level::warn, "Invalid payload size for orientation package: {}", payload_size);
        return;
    }
    int num_datapoints = payload_size / 2;

    std::vector<int> accel_channels = board_descr["accel_channels"];
    std::vector<int> gyro_channels = board_descr["gyro_channels"];
    std::vector<int> other_channels = board_descr["other_channels"];

    for (int i = 0; i < num_datapoints; i++)
    {
        double data = cast_16bit_to_int32 ((unsigned char *)(payload + 2 * i));
        if (i < 3)
        {
            package[accel_channels[i]] = 0.061 * data;
        }
        else if (i < 6)
        {
            package[gyro_channels[i - 3]] = 8.750 * data;
        }
        else
        {
            package[other_channels[i - 6]] = 1.52 * data;
            if (i == 6)
            {
                data *= -1; // no idea why, copypaste
            }
        }
    }
}

void Explore::parse_eeg_data (
    double *package, char *payload, int payload_size, double vref, int n_packages)
{
    if ((payload[payload_size - 4] != 0xAF) || (payload[payload_size - 3] != 0xBE) ||
        (payload[payload_size - 2] != 0xAD) || (payload[payload_size - 1] != 0xDE))
    {
        safe_logger (spdlog::level::warn, "checksum failed, {} {} {} {}", payload[payload_size - 4],
            payload[payload_size - 3], payload[payload_size - 2], payload[payload_size - 1]);
        return;
    }
    payload_size = payload_size - 4;
    if (payload_size % 3 != 0)
    {
        safe_logger (spdlog::level::warn, "Invalid payload size for EEG package: {}", payload_size);
        return;
    }
    int num_datapoints = payload_size / 3;
    if (num_datapoints % n_packages != 0)
    {
        safe_logger (spdlog::level::warn,
            "Invalid payload size or n_packages for EEG package: {}, {}", payload_size, n_packages);
        return;
    }
    std::vector<int> eeg_channels = board_descr["eeg_channels"];
    if (num_datapoints % eeg_channels.size () != 0)
    {
        safe_logger (spdlog::level::warn, "Invalid payload size for num_eeg_channels: {}, {}",
            payload_size, eeg_channels.size ());
        return;
    }

    // convert to uV
    std::vector<double> data;
    for (int i = 0; i < num_datapoints; i++)
    {
        double gain = 1.E-6 * (pow (2, 23) - 1) * 6.0;
        double datapoint = (double)cast_24bit_to_int32 ((unsigned char *)(payload + i * 3));
        datapoint = datapoint * vref / gain;
        data.push_back (datapoint);
    }
    // submit packages
    for (int i = 0; i < n_packages; i++)
    {
        for (int j = 0; j < (int)eeg_channels.size (); j++)
        {
            package[eeg_channels[j]] = data[i * eeg_channels.size () + j];
        }
        push_package (package);
    }
}
