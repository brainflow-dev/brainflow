#include "neurosky.h"
#include "thinkgear_parser.h"
#include "timestamp.h"

#include <chrono>

NeuroSky::NeuroSky (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::NEUROSKY_BOARD, params), keep_alive (false), ready (false)
{
}

NeuroSky::~NeuroSky ()
{
    skip_logs = true;
    release_session ();
}

int NeuroSky::prepare_session ()
{
    if (serial)
        return (int)BrainFlowExitCodes::STATUS_OK;
    if (params.serial_port.empty () || (params.timeout < 0) || (params.timeout > 600))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    // A pre-paired serial/RFCOMM port is required. No proprietary SDK is loaded.
    int baud = 57600;
    if (!params.other_info.empty ())
    {
        if (params.other_info == "115200")
            baud = 115200;
        else if (params.other_info != "57600")
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    serial.reset (Serial::create (params.serial_port.c_str (), this));
    if (serial->open_serial_port () != SerialExitCodes::OK)
    {
        serial.reset ();
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    if ((serial->set_serial_port_settings (100) != SerialExitCodes::OK) ||
        (serial->set_custom_baudrate (baud) != SerialExitCodes::OK))
    {
        serial->close_serial_port ();
        serial.reset ();
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int NeuroSky::start_stream (int buffer_size, const char *streamer_params)
{
    if (!serial)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    int result = prepare_for_acquisition (buffer_size, streamer_params);
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
        return result;
    serial->flush_buffer ();
    ready = false;
    keep_alive = true;
    worker = std::thread (&NeuroSky::read_thread, this);
    std::unique_lock<std::mutex> lock (mutex);
    bool success =
        received.wait_for (lock, std::chrono::seconds (params.timeout ? params.timeout : 5),
            [this] { return ready || !keep_alive; });
    success = success && ready;
    lock.unlock ();
    if (!success)
    {
        stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void NeuroSky::read_thread ()
{
    thinkgear::Parser parser;
    uint32_t counter = 0;
    while (keep_alive)
    {
        uint8_t byte = 0;
        // Read one byte: Windows serial timeouts also depend on requested length.
        int size = serial->read_from_serial_port (&byte, 1);
        if (size < 0)
            break;
        if (size == 0)
            continue;
        auto samples = parser.feed (byte);
        double timestamp = get_timestamp ();
        for (size_t i = 0; i < samples.size (); ++i)
        {
            double package[15] = {0};
            package[0] = counter++;
            package[1] = samples[i].eeg;
            for (size_t channel = 0; channel < samples[i].auxiliary.size (); ++channel)
                package[channel + 2] = samples[i].auxiliary[channel];
            package[13] = timestamp - (samples.size () - 1 - i) / 512.0;
            push_package (package);
        }
        if (!samples.empty ())
        {
            std::lock_guard<std::mutex> lock (mutex);
            ready = true;
            received.notify_all ();
        }
    }
    keep_alive = false;
    received.notify_all ();
}

int NeuroSky::stop_stream ()
{
    if (!worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    keep_alive = false;
    worker.join ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int NeuroSky::release_session ()
{
    if (worker.joinable ())
        stop_stream ();
    if (serial)
    {
        serial->close_serial_port ();
        serial.reset ();
    }
    free_packages ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int NeuroSky::config_board (std::string, std::string &)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
