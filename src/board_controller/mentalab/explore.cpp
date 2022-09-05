#include <string.h>
#include <vector>

#include "explore.h"

#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"

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

int Explore::prepare_session ()
{
    if (params.ip_port <= 0)
    {
        params.ip_port = 5; // default for explore
    }
    return BTLibBoard::prepare_session ();
}

int Explore::start_stream (int buffer_size, const char *streamer_params)
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

    res = bluetooth_open_device ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for the 1st package received
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    int num_secs = 7;
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
    // exg data
    int num_exg_rows = board_descr["default"]["num_rows"];
    double *package_exg = new double[num_exg_rows];
    memset (package_exg, 0, sizeof (double) * num_exg_rows);
    // orientation data
    int num_aux_rows = board_descr["auxiliary"]["num_rows"];
    double *package_aux = new double[num_aux_rows];
    memset (package_aux, 0, sizeof (double) * num_aux_rows);
    // env data
    int num_anc_rows = board_descr["ancillary"]["num_rows"];
    double *package_anc = new double[num_anc_rows];
    memset (package_anc, 0, sizeof (double) * num_anc_rows);

    unsigned char payload_buffer[UINT16_MAX];

    while (keep_alive)
    {
        struct ExploreHeader header;
        int res = bluetooth_get_data ((char *)&header, sizeof (header));
        if (res < 0)
        {
            safe_logger (spdlog::level::err, "error reading data from bluetooth");
        }
        if (res != 8)
        {
            continue;
        }
        // notify main thread that 1st byte received
        if (state != (int)BrainFlowExitCodes::STATUS_OK)
        {
            {
                std::lock_guard<std::mutex> lk (m);
                state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            cv.notify_one ();
            safe_logger (spdlog::level::debug, "received first packahe");
        }
        res = 0;
        // safe_logger (spdlog::level::trace, "header pid counter size timestamp: {} {} {} {}",
        // header.pid, header.counter, header.payload_size, header.timestamp);

        header.payload_size -= 4; // its because of timestamp which moved to header from the package
        if (header.payload_size < 1)
        {
            safe_logger (spdlog::level::err, "negative size for payload");
            continue;
        }

        while ((res != header.payload_size) && (keep_alive))
        {
            res = bluetooth_get_data ((char *)payload_buffer, header.payload_size);
        }
        if (!keep_alive)
        {
            break;
        }

        switch (header.pid)
        {
            case 0x0d:
                parse_orientation_data (&header, package_aux, payload_buffer);
                break;
            case 0x90: // eeg94
                parse_eeg_data (&header, package_exg, payload_buffer, 2.4, 33);
                break;
            case 0xD0: // eeg94r
                parse_eeg_data (&header, package_exg, payload_buffer, 2.4, 33);
                break;
            case 0x92: // eeg98
                parse_eeg_data (&header, package_exg, payload_buffer, 2.4, 16);
                break;
            case 0xD2: // eeg98r
                parse_eeg_data (&header, package_exg, payload_buffer, 2.4, 16);
                break;
            case 0x3e: // eeg99
                parse_eeg_data (&header, package_exg, payload_buffer, 4.5, 16);
                break;
            case 0x1e: // eeg99s
                parse_eeg_data (&header, package_exg, payload_buffer, 4.5, 16);
                break;
            case 0x13: // env
                parse_env_data (&header, package_anc, payload_buffer);
                break;
            default:
                break;
        }
    }
    delete[] package_anc;
    delete[] package_exg;
    delete[] package_aux;
}

std::string Explore::get_name_selector ()
{
    return "Explore";
}

void Explore::parse_orientation_data (
    const ExploreHeader *header, double *package, unsigned char *payload)
{
    int payload_size = header->payload_size;
    if ((payload[payload_size - 4] != 0xAF) || (payload[payload_size - 3] != 0xBE) ||
        (payload[payload_size - 2] != 0xAD) || (payload[payload_size - 1] != 0xDE))
    {
        safe_logger (spdlog::level::warn, "checksum failed, {} {} {} {}", payload[payload_size - 4],
            payload[payload_size - 3], payload[payload_size - 2], payload[payload_size - 1]);
        return;
    }
    payload_size = payload_size - 4;

    std::vector<int> accel_channels = board_descr["auxiliary"]["accel_channels"];
    std::vector<int> gyro_channels = board_descr["auxiliary"]["gyro_channels"];
    std::vector<int> other_channels = board_descr["auxiliary"]["other_channels"];

    if (payload_size % 2 != 0) // 2 is int16
    {
        safe_logger (spdlog::level::warn, "Invalid payload size for Orn package: {}");
        return;
    }
    int num_datapoints = payload_size / 2;

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
                data *= -1; // no idea why, copypaste, maybe bug in fw
            }
        }
    }
    package[board_descr["auxiliary"]["timestamp_channel"].get<int> ()] =
        get_timestamp (); // todo improve
    package[board_descr["auxiliary"]["package_num_channel"].get<int> ()] = header->counter;
    push_package (package, (int)BrainFlowPresets::AUXILIARY_PRESET);
}

void Explore::parse_eeg_data (const ExploreHeader *header, double *package, unsigned char *payload,
    double vref, int n_packages)
{
    int payload_size = header->payload_size;
    if ((payload[payload_size - 4] != 0xAF) || (payload[payload_size - 3] != 0xBE) ||
        (payload[payload_size - 2] != 0xAD) || (payload[payload_size - 1] != 0xDE))
    {
        safe_logger (spdlog::level::warn, "checksum failed, {} {} {} {}", payload[payload_size - 4],
            payload[payload_size - 3], payload[payload_size - 2], payload[payload_size - 1]);
        return;
    }
    payload_size = payload_size - 4;
    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    if ((payload_size % n_packages != 0) || (payload_size % 3 != 0)) // 3 is int24 format
    {
        safe_logger (spdlog::level::warn,
            "Invalid payload size for EEG package: {}, n_packages: {}", payload_size, n_packages);
        return;
    }
    if (payload_size % (eeg_channels.size () + 1) != 0) // 1 for data status, maybe its reference
    {
        safe_logger (spdlog::level::warn, "Invalid payload size for num_eeg_channels: {}, {}",
            payload_size, eeg_channels.size ());
        return;
    }

    int num_datapoints = payload_size / 3;
    // convert to uV
    std::vector<double> data;
    for (int i = 0; i < num_datapoints; i++)
    {
        double gain = 1.E-6 * (pow (2, 23) - 1) * 6.0;
        double datapoint = (double)cast_24bit_to_int32 ((unsigned char *)(payload + i * 3));
        datapoint = datapoint * vref / gain;
        data.push_back (datapoint);
    }
    int num_total_channels = (int)eeg_channels.size () + 1;
    int other_channel = board_descr["default"]["other_channels"][0];
    // submit packages
    for (int i = 0; i < n_packages; i++)
    {
        for (int j = 0; j < num_total_channels; j++)
        {
            if (j == 0)
                package[other_channel] = data[i * num_total_channels + j];
            else
                package[eeg_channels[j - 1]] = data[i * num_total_channels + j];
        }
        package[board_descr["default"]["timestamp_channel"].get<int> ()] =
            get_timestamp (); // todo improve
        package[board_descr["default"]["package_num_channel"].get<int> ()] = header->counter;
        push_package (package, (int)BrainFlowPresets::DEFAULT_PRESET);
    }
}

void Explore::parse_env_data (const ExploreHeader *header, double *package, unsigned char *payload)
{
    int payload_size = header->payload_size;
    if ((payload[payload_size - 4] != 0xAF) || (payload[payload_size - 3] != 0xBE) ||
        (payload[payload_size - 2] != 0xAD) || (payload[payload_size - 1] != 0xDE))
    {
        safe_logger (spdlog::level::warn, "checksum failed, {} {} {} {}", payload[payload_size - 4],
            payload[payload_size - 3], payload[payload_size - 2], payload[payload_size - 1]);
        return;
    }
    payload_size = payload_size - 4;
    if (payload_size < 5)
    {
        safe_logger (spdlog::level::warn, "invalid size for env package: {}", payload_size);
        return;
    }
    int temperature_channel = board_descr["ancillary"]["temperature_channels"][0];
    int battery_channel = board_descr["ancillary"]["battery_channel"];
    int other_channel = board_descr["ancillary"]["other_channels"][0];

    double battery =
        (16.8 / 6.8) * (1.8 / 2457) * cast_16bit_to_int32 ((unsigned char *)(payload + 3));
    package[temperature_channel] = (double)payload[0];
    package[battery_channel] = get_battery_percentage (battery);
    package[other_channel] =
        (1000.0 / 4095.0) * cast_16bit_to_int32 ((unsigned char *)(payload + 1));
    package[board_descr["ancillary"]["timestamp_channel"].get<int> ()] =
        get_timestamp (); // todo improve
    package[board_descr["ancillary"]["package_num_channel"].get<int> ()] = header->counter;
    push_package (package, (int)BrainFlowPresets::ANCILLARY_PRESET);
}

double Explore::get_battery_percentage (double battery)
{
    double percentage = 0.0;
    if (battery < 3.1)
        percentage = 1;
    else if (battery < 3.5)
        percentage = 1 + (battery - 3.1) / .4 * 10;
    else if (battery < 3.8)
        percentage = 10 + (battery - 3.5) / .3 * 40;
    else if (battery < 3.9)
        percentage = 40 + (battery - 3.8) / .1 * 20;
    else if (battery < 4.)
        percentage = 60 + (battery - 3.9) / .1 * 15;
    else if (battery < 4.1)
        percentage = 75 + (battery - 4.) / .1 * 15;
    else if (battery < 4.2)
        percentage = 90 + (battery - 4.1) / .1 * 10;
    else if (battery > 4.2)
        percentage = 100;

    return percentage;
}