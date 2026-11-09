#include "brainvision_rda.h"
#include "timestamp.h"

#include <chrono>

BrainVisionRDA::BrainVisionRDA (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::BRAINVISION_RDA_BOARD, params)
    , keep_alive (false)
    , stream_error ((int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR)
    , initialized (false)
    , ready (false)
{
}

BrainVisionRDA::~BrainVisionRDA ()
{
    skip_logs = true;
    release_session ();
}

bool BrainVisionRDA::connect_socket ()
{
    socket.reset (
        new SocketClientTCP (params.ip_address.c_str (), params.ip_port ? params.ip_port : 51244));
    if (socket->connect () != (int)SocketClientTCPReturnCodes::STATUS_OK)
    {
        socket.reset ();
        return false;
    }
    return true;
}

int BrainVisionRDA::prepare_session ()
{
    if (initialized)
        return (int)BrainFlowExitCodes::STATUS_OK;
    struct in_addr address;
    if ((params.ip_address.size () > 31) ||
        (inet_pton (AF_INET, params.ip_address.c_str (), &address) != 1) || (params.ip_port < 0) ||
        (params.ip_port > 65535) || (params.timeout < 0) || (params.timeout > 600))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    if (!connect_socket ())
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainVisionRDA::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    int result = prepare_for_acquisition (buffer_size, streamer_params);
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
        return result;
    // Reconnect on every start to obtain fresh START metadata and discard old queued data.
    socket.reset ();
    if (!connect_socket ())
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    ready = false;
    channel_names.clear ();
    markers.clear ();
    stream_error = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    keep_alive = true;
    worker = std::thread (&BrainVisionRDA::read_thread, this);
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

bool BrainVisionRDA::read_exact (void *data, size_t size)
{
    size_t offset = 0;
    while (keep_alive && (offset < size))
    {
        int count = socket->recv (static_cast<uint8_t *> (data) + offset, int (size - offset));
        if (count <= 0)
            return false;
        offset += size_t (count);
    }
    return offset == size;
}

void BrainVisionRDA::read_thread ()
{
    rda::Decoder decoder;
    bool has_block = false;
    uint32_t previous_block = 0;
    while (keep_alive)
    {
        uint8_t header[24];
        uint32_t size, type;
        if (!read_exact (header, sizeof (header)) || !rda::header (header, size, type))
            break;
        std::vector<uint8_t> body (size - sizeof (header));
        if (!read_exact (body.data (), body.size ()))
            break;
        if (type == 1)
        {
            if (!decoder.start (body))
            {
                safe_logger (spdlog::level::err,
                    "RDA requires 32 channels at 1000 Hz with valid resolutions");
                break;
            }
            std::lock_guard<std::mutex> lock (mutex);
            channel_names = decoder.names;
            has_block = false;
        }
        else if (type == 4)
        {
            rda::Block block;
            if (!decoder.data32 (body, block))
                break;
            if (has_block && (block.number != uint32_t (previous_block + 1)))
                safe_logger (spdlog::level::warn, "RDA block discontinuity: {} -> {}",
                    previous_block, block.number);
            has_block = true;
            previous_block = block.number;
            double timestamp = get_timestamp ();
            for (size_t i = 0; i < block.samples.size (); ++i)
            {
                block.samples[i][35] = timestamp - (block.samples.size () - 1 - i) / 1000.0;
                push_package (block.samples[i].data ());
            }
            std::lock_guard<std::mutex> lock (mutex);
            for (const auto &marker : block.markers)
            {
                if (markers.size () == 256)
                {
                    markers.pop_front ();
                    safe_logger (
                        spdlog::level::warn, "RDA marker queue full; oldest event discarded");
                }
                markers.push_back (marker);
            }
            ready = true;
            received.notify_all ();
        }
        else if ((type == 3) || (type == 2))
        {
            safe_logger (
                spdlog::level::info, "RDA stopped or unsupported DATA16 stream (type {})", type);
            break;
        }
        // Recorder state, INFO, impedance and keep-alive messages do not contain EEG.
    }
    stream_error = (int)BrainFlowExitCodes::INCOMMING_MSG_ERROR;
    keep_alive = false;
    received.notify_all ();
}

int BrainVisionRDA::stop_stream ()
{
    if (!worker.joinable ())
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    keep_alive = false;
    socket->shutdown ();
    worker.join ();
    socket.reset ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainVisionRDA::release_session ()
{
    if (worker.joinable ())
        stop_stream ();
    socket.reset ();
    initialized = false;
    free_packages ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainVisionRDA::config_board (std::string config, std::string &response)
{
    if (!initialized)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    std::lock_guard<std::mutex> lock (mutex);
    if (config == "get_channel_names")
        response = json (channel_names).dump ();
    else if (config == "get_marker")
    {
        if (markers.empty ())
            response = "{}";
        else
        {
            auto marker = markers.front ();
            response = json ({{"block", marker.block}, {"position", marker.position},
                                 {"points", marker.points}, {"channel", marker.channel},
                                 {"type", marker.type}, {"description", marker.description}})
                           .dump ();
            markers.pop_front ();
        }
    }
    else
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
