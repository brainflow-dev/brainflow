#include <stdint.h>
#include <string.h>

#include "notion_osc.h"

#ifndef _WIN32
#include <errno.h>
#endif

#include <iostream>

constexpr int NotionOSC::num_channels;


NotionOSC::NotionOSC (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::NOTION_OSC_BOARD, params)
{
    socket = NULL;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

NotionOSC::~NotionOSC ()
{
    skip_logs = true;
    release_session ();
}

int NotionOSC::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    int port = 9000;
    if (params.ip_port != 0)
    {
        port = params.ip_port;
    }
    safe_logger (spdlog::level::debug, "Use IP port {}", port);

    socket = new BroadCastClient (port);
    int res = socket->init ();
    if (res != (int)BroadCastClientReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        delete socket;
        socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int NotionOSC::config_board (char *config)
{
    safe_logger (spdlog::level::err, "Notion OSC doesnt support config_board");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int NotionOSC::start_stream (int buffer_size, char *streamer_params)
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
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }
    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (NotionOSC::num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        delete db;
        db = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 5 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        return state;
    }
    else
    {
        Board::board_logger->error ("no data received in 5sec, stopping thread");
        this->stop_stream ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
}

int NotionOSC::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int NotionOSC::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        initialized = false;
        if (socket)
        {
            socket->close ();
            delete socket;
            socket = NULL;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void NotionOSC::read_thread ()
{
    int res;
    constexpr int max_package_size = 8192;
    unsigned char b[max_package_size];
    while (keep_alive)
    {
        res = socket->recv (b, max_package_size);
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
            continue;
        }
        else
        {
            if (state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::info,
                    "received first package with {} bytes streaming is started", res);
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
            try
            {
                handle_packet (OSCPP::Server::Packet (b, res));
            }
            catch (...)
            {
                // do nothing
            }
        }
    }
}

void NotionOSC::handle_packet (const OSCPP::Server::Packet &packet)
{
    if (packet.isBundle ())
    {
        OSCPP::Server::Bundle bundle (packet);
        OSCPP::Server::PacketStream packets (bundle.packets ());
        while (!packets.atEnd ())
        {
            handle_packet (packets.next ());
        }
    }
    else
    {
        double package[NotionOSC::num_channels] = {0.0};

        OSCPP::Server::Message msg (packet);
        std::string msg_address = std::string (msg.address ());
        OSCPP::Server::ArgStream args (msg.args ());
        if ((msg_address.size () > 3) &&
            (msg_address.compare (msg_address.size () - 3, 3, "raw") == 0))
        {
            // check serial number if provided
            if (!params.serial_number.empty ())
            {
                if (msg_address.find (params.serial_number) == std::string::npos)
                {
                    safe_logger (spdlog::level::trace,
                        "found package from different device. Check provided serial number");
                    return;
                }
            }
            // parse data
            try
            {
                // eeg
                OSCPP::Server::ArgStream eeg_data (args.array ());
                int counter = 0;
                while (!eeg_data.atEnd ())
                {
                    package[1 + counter] = (double)eeg_data.float32 ();
                    counter++;
                }
                if (counter != 8)
                {
                    safe_logger (spdlog::level::trace,
                        "wrong format for eeg data, must be 8 values, found {}", counter);
                }
                std::string timestamp_str = args.string ();
                double timestamp = std::stod (timestamp_str);
                // package num
                package[0] = (double)args.int32 ();
                // marker
                std::string marker = std::string (args.string ());
                if (!marker.empty ())
                {
                    try
                    {
                        package[9] = std::stod (marker);
                    }
                    catch (...)
                    {
                        safe_logger (
                            spdlog::level::err, "For BrainFlow marker should be numeric value.");
                    }
                }
                // commit package
                db->add_data (timestamp, package);
                streamer->stream_data (package, NotionOSC::num_channels, timestamp);
            }
            catch (std::exception &e)
            {
                safe_logger (
                    spdlog::level::trace, "Exception in parsing OSC packet: {}", e.what ());
                return;
            }
        }
        else
        {
            safe_logger (spdlog::level::trace, "Unknown msg: {}", msg_address.c_str ());
        }
    }
}
