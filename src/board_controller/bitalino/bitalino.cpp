#include "bitalino.h"
#include "bitalino_parser.h"
#include "timestamp.h"

#include <chrono>

BITalino::BITalino (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::BITALINO_BOARD, params), keep_alive (false), ready (false)
{
}

BITalino::~BITalino ()
{
    skip_logs = true;
    release_session ();
}

bool BITalino::send_command (unsigned char command)
{
    // The firmware needs a gap between commands (also used by the vendor API).
    std::this_thread::sleep_for (std::chrono::milliseconds (150));
    return serial->send_to_serial_port (&command, 1) == 1;
}

bool BITalino::read_version ()
{
    if (!send_command (0x07))
        return false;
    const std::string prefix = "BITalino";
    std::string response;
    auto deadline = std::chrono::steady_clock::now () +
        std::chrono::seconds (params.timeout ? params.timeout : 5);
    while (std::chrono::steady_clock::now () < deadline)
    {
        char byte;
        int size = serial->read_from_serial_port (&byte, 1);
        if (size < 0)
            return false;
        if (size == 0)
            continue;
        if (response.size () < prefix.size ())
        {
            if (byte == prefix[response.size ()])
                response += byte;
            else
                response = (byte == 'B') ? "B" : "";
        }
        else if (byte == '\n')
        {
            version = response;
            return true;
        }
        else
        {
            response += byte;
            if (response.size () > 256)
                return false;
        }
    }
    return false;
}

int BITalino::prepare_session ()
{
    if (serial)
        return (int)BrainFlowExitCodes::STATUS_OK;
    if (params.serial_port.empty () || (params.timeout < 0) || (params.timeout > 600) ||
        (!params.other_info.empty () && (params.other_info != "simulated")))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    serial.reset (Serial::create (params.serial_port.c_str (), this));
    if (serial->open_serial_port () != SerialExitCodes::OK)
    {
        serial.reset ();
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    if ((serial->set_serial_port_settings (100) != SerialExitCodes::OK) ||
        (serial->set_custom_baudrate (115200) != SerialExitCodes::OK))
    {
        release_session ();
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    if (!send_command (0x00) || !read_version ())
    {
        release_session ();
        return (int)BrainFlowExitCodes::INITIAL_MSG_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::start_stream (int buffer_size, const char *streamer_params)
{
    if (!serial)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    int result = prepare_for_acquisition (buffer_size, streamer_params);
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
        return result;
    serial->flush_buffer ();
    if (!send_command (0xc3) || !send_command (params.other_info == "simulated" ? 0xfe : 0xfd))
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    ready = false;
    keep_alive = true;
    worker = std::thread (&BITalino::read_thread, this);
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

void BITalino::read_thread ()
{
    bitalino::Parser parser;
    while (keep_alive)
    {
        uint8_t byte = 0;
        int size = serial->read_from_serial_port (&byte, 1);
        if (size < 0)
            break;
        if (size == 0)
            continue;
        auto samples = parser.feed (byte);
        double timestamp = get_timestamp ();
        for (size_t i = 0; i < samples.size (); ++i)
        {
            double package[13] = {0};
            for (size_t channel = 0; channel < samples[i].size (); ++channel)
                package[channel] = samples[i][channel];
            package[11] = timestamp - (samples.size () - 1 - i) / 1000.0;
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

int BITalino::stop_stream ()
{
    if (!worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    keep_alive = false;
    worker.join ();
    if (!send_command (0x00) || !read_version ())
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BITalino::release_session ()
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

int BITalino::config_board (std::string config, std::string &response)
{
    if (!serial)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (config != "get_version")
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    response = version;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
