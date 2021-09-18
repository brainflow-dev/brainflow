#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "ironbci.h"
#include "serial.h"
#include "timestamp.h"

constexpr int IronBCI::ads_gain;
constexpr int IronBCI::start_byte;
constexpr int IronBCI::stop_byte;
const std::string IronBCI::start_command = "b";
const std::string IronBCI::stop_command = "s";

#ifdef USE_PERIPHERY
IronBCI::IronBCI (struct BrainFlowInputParams params) : Board ((int)BoardIds::IRONBCI_BOARD, params)
{
    spi = NULL;
    keep_alive = false;
    initialized = false;
}

IronBCI::~IronBCI ()
{
    skip_logs = true;
    release_session ();
}

int IronBCI::config_board (std::string config, std::string &response)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "Serial port is empty");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    spi = spi_new ();
    // mode 0, max speed 1mhz
    if (spi_open (spi, params.serial_port.c_str (), 0, 1000000) < 0)
    {
        spi_free (spi);
        spi = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::start_stream (int buffer_size, const char *streamer_params)
{
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

    // todo send smth to start streaming ?

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }
        // todo send smth to stop streaming?
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int IronBCI::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
    }
    if (spi)
    {
        spi_close (spi);
        spi_free (spi);
        spi = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void IronBCI::read_thread ()
{
    int res;
    unsigned char b[26];
    float eeg_scale = 4.5 / float ((pow (2, 23) - 1)) / IronBCI::ads_gain * 1000000.;
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    while (keep_alive)
    {
        // todo parse data

        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
    }
    delete[] package;
}

#else

IronBCI::IronBCI (struct BrainFlowInputParams params) : Board ((int)BoardIds::IRONBCI_BOARD, params)
{
}

IronBCI::~IronBCI ()
{
}

int IronBCI::prepare_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::release_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::stop_stream ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::start_stream (int buffer_size, char *streamer_params)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
