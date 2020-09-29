#include <chrono>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "playback_file_board.h"

#define SET_LOOPBACK_TRUE "loopback_true"
#define SET_LOOPBACK_FALSE "loopback_false"


PlaybackFileBoard::PlaybackFileBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::PLAYBACK_FILE_BOARD,
          params) // its a hack - set board_id for playback board here temporary and override it
                  // with master board id in prepare_session, board_id is protected and there is no
                  // api to get it so its ok
{
    keep_alive = false;
    loopback = false;
    is_streaming = false;
    initialized = false;
    package_size = 0;
    this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

PlaybackFileBoard::~PlaybackFileBoard ()
{
    skip_logs = true;
    release_session ();
}

int PlaybackFileBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if ((params.file.empty ()) || (params.other_info.empty ()))
    {
        safe_logger (spdlog::level::err, "playback file or master board id not provided");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        board_id = std::stoi (params.other_info);
    }
    catch (const std::exception &e)
    {
        safe_logger (
            spdlog::level::err, "Write board id of board which recorded data to other_info field");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    // check that file exist in prepare_session
    FILE *fp;
    fp = fopen (params.file.c_str (), "r");
    if (fp == NULL)
    {
        safe_logger (spdlog::level::err, "invalid file path");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    fclose (fp);

    // get package size for master board
    int res = get_num_rows (board_id, &package_size);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int PlaybackFileBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::trace, "start stream");
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }
    if (db)
    {
        delete db;
        db = NULL;
    }
    int res = prepare_streamer (streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (package_size - 1, buffer_size); // - 1 because of timestamp
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 2 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 2sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int PlaybackFileBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int PlaybackFileBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();

        if (db)
        {
            delete db;
            db = NULL;
        }
        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void PlaybackFileBoard::read_thread ()
{
    FILE *fp;
    fp = fopen (params.file.c_str (), "r");
    if (fp == NULL)
    {
        safe_logger (spdlog::level::err, "failed to open file in thread");
        return;
    }
    double *package = new double[package_size];
    char buf[4096];
    double last_timestamp = -1.0;

    while (keep_alive)
    {
        char *res = fgets (buf, sizeof (buf), fp);
        if ((loopback) && (res == NULL))
        {
            fseek (fp, 0, SEEK_SET); // go to beginning'
            last_timestamp = -1.0;
            continue;
        }
        if ((!loopback) && (res == NULL))
        {
// busy wait instead exit
#ifdef _WIN32
            Sleep (1);
#else
            usleep (1000);
#endif
            continue;
        }
        // res not NULL
        std::string csv_string (buf);
        std::stringstream ss (csv_string);
        std::vector<std::string> splitted;
        std::string tmp;
        while (getline (ss, tmp, ','))
        {
            splitted.push_back (tmp);
        }
        if (splitted.size () != package_size)
        {
            safe_logger (spdlog::level::err,
                "invalid string in file, check provided board id. String size {}, expected size {}",
                splitted.size (), package_size);
            continue;
        }
        for (int i = 0; i < package_size; i++)
        {
            package[i] = std::stod (splitted[i]);
        }
        // notify main thread
        if (this->state != (int)BrainFlowExitCodes::STATUS_OK)
        {
            {
                std::lock_guard<std::mutex> lk (this->m);
                this->state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            this->cv.notify_one ();
        }
        streamer->stream_data (
            package, package_size - 1, package[package_size - 1]); // - 1 because of timestamp
        db->add_data (package[package_size - 1], package);
        if (last_timestamp > 0)
        {
            double time_wait = package[package_size - 1] - last_timestamp; // in seconds
#ifdef _WIN32
            Sleep ((int)(time_wait * 1000 + 0.5));
#else
            usleep ((int)(time_wait * 1000000 + 0.5));
#endif
        }
        last_timestamp = package[package_size - 1];
    }
    fclose (fp);
    delete[] package;
}

int PlaybackFileBoard::config_board (char *config)
{
    if (strcmp (config, SET_LOOPBACK_TRUE) == 0)
    {
        loopback = true;
    }
    else if (strcmp (config, SET_LOOPBACK_FALSE) == 0)
    {
        loopback = false;
    }
    else
    {
        safe_logger (spdlog::level::warn, "invalid config string {}", config);
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}
