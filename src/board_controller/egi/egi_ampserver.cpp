#include "egi_ampserver.h"
#include "egi_protocol.h"
#include "timestamp.h"

#include <chrono>
#include <vector>

EGIAmpServer::EGIAmpServer (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::EGI_AMP_SERVER_BOARD, params)
    , keep_alive (false)
    , stream_error ((int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR)
    , ready (false)
    , owns_stream (false)
    , amplifier (0)
{
}

EGIAmpServer::~EGIAmpServer ()
{
    skip_logs = true;
    release_session ();
}

bool EGIAmpServer::send_all (SocketClientTCP &socket, const std::string &message)
{
    size_t offset = 0;
    while (offset < message.size ())
    {
        int count = socket.send (message.data () + offset, int (message.size () - offset));
        if (count <= 0)
            return false;
        offset += size_t (count);
    }
    return true;
}

bool EGIAmpServer::command (
    const std::string &name, const std::string &value, std::string &response)
{
    response.clear ();
    std::string message =
        "(sendCommand " + name + " " + std::to_string (amplifier) + " 0 " + value + ")\n";
    if (!send_all (*commands, message))
        return false;
    int depth = 0;
    auto deadline = std::chrono::steady_clock::now () + std::chrono::seconds (5);
    while ((response.size () < 8192) && (std::chrono::steady_clock::now () < deadline))
    {
        char byte;
        if (commands->recv (&byte, 1) != 1)
            return false;
        if (response.empty () && std::isspace (static_cast<unsigned char> (byte)))
            continue;
        if (response.empty () && (byte != '('))
            return false;
        response += byte;
        if (byte == '(')
            ++depth;
        else if (byte == ')')
            --depth;
        if (depth == 0)
            return response.find ("(sendCommand_return") == 0 &&
                egi::field (response, "status") == "complete";
    }
    return false;
}

int EGIAmpServer::prepare_session ()
{
    if (commands)
        return (int)BrainFlowExitCodes::STATUS_OK;
    struct in_addr address;
    if ((params.ip_address.size () > 31) ||
        (inet_pton (AF_INET, params.ip_address.c_str (), &address) != 1) || (params.ip_port < 0) ||
        (params.ip_port > 65535) || (params.ip_port_aux < 0) || (params.ip_port_aux > 65535) ||
        (params.timeout < 0) || (params.timeout > 600))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    amplifier = 0;
    if (!params.other_info.empty ())
    {
        // Numeric amplifier ID, not a command string.
        if ((params.other_info.size () > 4) ||
            (params.other_info.find_first_not_of ("0123456789") != std::string::npos))
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        amplifier = std::stoi (params.other_info);
    }
    commands.reset (
        new SocketClientTCP (params.ip_address.c_str (), params.ip_port ? params.ip_port : 9877));
    if (commands->connect () != (int)SocketClientTCPReturnCodes::STATUS_OK)
    {
        commands.reset ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (!command ("cmd_GetAmpDetails", "0", details) ||
        (egi::field (details, "amp_type") != "NA400") ||
        (egi::field (details, "packet_format") != "2") ||
        (egi::field (details, "number_of_channels") != "256"))
    {
        safe_logger (spdlog::level::err, "EGI requires NA400, Packet Format 2, 256 channels");
        commands.reset ();
        return (int)BrainFlowExitCodes::INITIAL_MSG_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int EGIAmpServer::start_stream (int buffer_size, const char *streamer_params)
{
    if (!commands)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    int result = prepare_for_acquisition (buffer_size, streamer_params);
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
        return result;
    data_socket.reset (new SocketClientTCP (
        params.ip_address.c_str (), params.ip_port_aux ? params.ip_port_aux : 9879));
    if (data_socket->connect () != (int)SocketClientTCPReturnCodes::STATUS_OK)
    {
        data_socket.reset ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    std::string response;
    if (!command ("cmd_Stop", "0", response) || !command ("cmd_SetPower", "0", response) ||
        !command ("cmd_SetDecimatedRate", "1000", response) ||
        !command ("cmd_SetPower", "1", response) ||
        !command ("cmd_DefaultAcquisitionState", "0", response) ||
        !command ("cmd_Start", "0", response))
    {
        data_socket.reset ();
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    owns_stream = true;
    if (!send_all (
            *data_socket, "(sendCommand cmd_ListenToAmp " + std::to_string (amplifier) + " 0 0)\n"))
    {
        command ("cmd_Stop", "0", response);
        owns_stream = false;
        data_socket.reset ();
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    ready = false;
    stream_error = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    keep_alive = true;
    worker = std::thread (&EGIAmpServer::read_thread, this);
    std::unique_lock<std::mutex> lock (mutex);
    bool success =
        received.wait_for (lock, std::chrono::seconds (params.timeout ? params.timeout : 5),
            [this] { return ready || !keep_alive; });
    success = success && ready;
    lock.unlock ();
    if (!success)
    {
        int error = stream_error;
        stop_stream ();
        return error;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

bool EGIAmpServer::read_exact (void *data, size_t size)
{
    size_t offset = 0;
    while (keep_alive && (offset < size))
    {
        int count = data_socket->recv (static_cast<uint8_t *> (data) + offset, int (size - offset));
        if (count <= 0)
            return false;
        offset += size_t (count);
    }
    return offset == size;
}

void EGIAmpServer::read_thread ()
{
    bool first = true;
    uint64_t previous = 0;
    while (keep_alive)
    {
        uint8_t header[16];
        size_t size;
        if (!read_exact (header, sizeof (header)) ||
            !egi::header (header, uint64_t (amplifier), size))
            break;
        std::vector<uint8_t> body (size);
        if (!read_exact (body.data (), body.size ()))
            break;
        std::vector<egi::Sample> samples;
        bool valid = true;
        for (size_t offset = 0; offset < size; offset += 1264)
        {
            egi::Sample sample;
            if (!egi::decode (body.data () + offset, 1264, sample))
            {
                valid = false;
                break;
            }
            if (!first && sample.counter == previous)
                continue;
            if (!first && sample.counter != previous + 1)
                safe_logger (spdlog::level::warn, "EGI sample discontinuity: {} -> {}", previous,
                    sample.counter);
            previous = sample.counter;
            first = false;
            samples.push_back (sample);
        }
        if (!valid)
            break;
        double timestamp = get_timestamp ();
        for (size_t i = 0; i < samples.size (); ++i)
        {
            samples[i].values[262] = timestamp - (samples.size () - 1 - i) / 1000.0;
            push_package (samples[i].values.data ());
        }
        if (!samples.empty ())
        {
            std::lock_guard<std::mutex> lock (mutex);
            ready = true;
            received.notify_all ();
        }
    }
    stream_error = (int)BrainFlowExitCodes::INCOMMING_MSG_ERROR;
    keep_alive = false;
    received.notify_all ();
}

int EGIAmpServer::stop_stream ()
{
    if (!worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    keep_alive = false;
    data_socket->shutdown ();
    worker.join ();
    data_socket.reset ();
    std::string response;
    bool stopped = !owns_stream || command ("cmd_Stop", "0", response);
    owns_stream = false;
    return stopped ? (int)BrainFlowExitCodes::STATUS_OK :
                     (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
}

int EGIAmpServer::release_session ()
{
    if (worker.joinable ())
        stop_stream ();
    data_socket.reset ();
    commands.reset ();
    free_packages ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int EGIAmpServer::config_board (std::string config, std::string &response)
{
    if (!commands)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (config != "get_amp_details")
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    response = details;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
