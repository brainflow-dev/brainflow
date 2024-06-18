#include <algorithm>
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
#include "timestamp.h"

#define SET_LOOPBACK_TRUE "loopback_true"
#define SET_LOOPBACK_FALSE "loopback_false"
#define NEW_TIMESTAMPS "new_timestamps"
#define OLD_TIMESTAMPS "old_timestamps"
#define SET_INDEX_PREFIX "set_index_percentage:"
#define MAX_LINE_LENGTH 8192


PlaybackFileBoard::PlaybackFileBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::PLAYBACK_FILE_BOARD,
          params) // its a hack - set board_id for playback board here temporary and override it
                  // with master board id in prepare_session, board_id is protected and there is no
                  // api to get it so its ok
{
    keep_alive = false;
    loopback = false;
    initialized = false;
    use_new_timestamps = true;
    pos_percentage.resize (3);
    std::fill (pos_percentage.begin (), pos_percentage.end (), -1);
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

    if (params.master_board == (int)BoardIds::NO_BOARD)
    {
        safe_logger (spdlog::level::err, "master board id is not provided");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    try
    {
        board_id = params.master_board;
        board_descr = boards_struct.brainflow_boards_json["boards"][std::to_string (board_id)];
    }
    catch (json::exception &e)
    {
        safe_logger (spdlog::level::err, "invalid json for master board");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    catch (const std::exception &e)
    {
        safe_logger (
            spdlog::level::err, "Write board id of board which recorded data to other_info field");
        safe_logger (spdlog::level::err, e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (!params.file.empty ())
    {
        std::vector<long int> offsets;
        int res = get_file_offsets (params.file, offsets);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        else
        {
            file_offsets.push_back (offsets);
        }
    }
    if (!params.file_aux.empty ())
    {
        std::vector<long int> offsets;
        int res = get_file_offsets (params.file_aux, offsets);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        else
        {
            file_offsets.push_back (offsets);
        }
    }
    if (!params.file_anc.empty ())
    {
        std::vector<long int> offsets;
        int res = get_file_offsets (params.file_anc, offsets);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        else
        {
            file_offsets.push_back (offsets);
        }
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int PlaybackFileBoard::start_stream (int buffer_size, const char *streamer_params)
{
    safe_logger (spdlog::level::trace, "start stream");
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

    keep_alive = true;
    if (!params.file.empty ())
    {
        streaming_threads.push_back (std::thread (
            [this] { this->read_thread ((int)BrainFlowPresets::DEFAULT_PRESET, params.file); }));
    }
    if (!params.file_aux.empty ())
    {
        streaming_threads.push_back (std::thread ([this]
            { this->read_thread ((int)BrainFlowPresets::AUXILIARY_PRESET, params.file_aux); }));
    }
    if (!params.file_anc.empty ())
    {
        streaming_threads.push_back (std::thread ([this]
            { this->read_thread ((int)BrainFlowPresets::ANCILLARY_PRESET, params.file_anc); }));
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int PlaybackFileBoard::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        for (std::thread &streaming_thread : streaming_threads)
        {
            streaming_thread.join ();
        }
        streaming_threads.clear ();
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
        free_packages ();
        initialized = false;
    }
    file_offsets.clear ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void PlaybackFileBoard::read_thread (int preset, std::string file)
{
    std::string preset_str = preset_to_string (preset);
    if (board_descr.find (preset_str) == board_descr.end ())
    {
        safe_logger (spdlog::level::err, "no preset {} for board {}", preset, board_id);
        return;
    }

    FILE *fp;
    fp = fopen (file.c_str (), "rb");
    if (fp == NULL)
    {
        safe_logger (spdlog::level::err, "failed to open file in thread");
        return;
    }

    json board_preset = board_descr[preset_str];
    int num_rows = board_preset["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    char buf[MAX_LINE_LENGTH];
    double last_timestamp = -1.0;
    bool new_timestamps = use_new_timestamps; // to prevent changing during streaming
    int timestamp_channel = board_preset["timestamp_channel"];
    double accumulated_time_delta = 0.0;

    bool reached_end = false;
    while (keep_alive)
    {
        auto start = std::chrono::high_resolution_clock::now ();
        // prevent race condition with another config_board method call
        lock.lock ();
        double cur_index = pos_percentage[preset];
        if ((int)cur_index >= 0)
        {
            int new_pos = (int)(cur_index * (file_offsets[preset].size () / 100.0));
            try
            {
                fseek (fp, file_offsets[preset][new_pos], SEEK_SET);
                safe_logger (spdlog::level::trace, "set position in a file to {}", new_pos);
            }
            catch (...)
            {
                // should never happen since input is already validated
                safe_logger (spdlog::level::warn, "invalid position in a file");
            }
            last_timestamp = -1;
            pos_percentage[preset] = -1;
        }
        lock.unlock ();
        char *res = fgets (buf, sizeof (buf), fp);
        if ((loopback) && (res == NULL))
        {
            fseek (fp, 0, SEEK_SET); // go to beginning'
            last_timestamp = -1.0;
            continue;
        }
        if ((!loopback) && (res == NULL))
        {
            if (!reached_end)
            {
                reached_end = true;
                // Log just once to inform the user that we have reached the endof the file, as we
                // stop sending any samples. The user-programmer could be waiting as the stream
                // needs to be stopped manually.
                safe_logger (
                    spdlog::level::trace, "End of file reached and not set to loop. Sleeping.");
            }
// busy wait instead exit
#ifdef _WIN32
            Sleep (1);
#else
            usleep (1000);
#endif
            continue;
        }
        // res not NULL
        std::string tsv_string (buf);
        std::stringstream ss (tsv_string);
        std::vector<std::string> splitted;
        std::string tmp;
        char sep = '\t';
        if (tsv_string.find ('\t') == std::string::npos)
        {
            sep = ',';
        }
        while (std::getline (ss, tmp, sep))
        {
            if (tmp != "\n")
            {
                splitted.push_back (tmp);
            }
        }
        if (splitted.size () != num_rows)
        {
            safe_logger (spdlog::level::err,
                "invalid string in file, check provided board id. String size {}, expected size {}",
                splitted.size (), num_rows);
            continue;
        }
        for (int i = 0; i < num_rows; i++)
        {
            try
            {
                package[i] = std::stod (splitted[i]);
            }
            catch (...)
            {
                safe_logger (spdlog::level::err, "failed to parse value: {}", splitted[i].c_str ());
            }
        }
        if (last_timestamp > 0)
        {
            double time_wait = (package[timestamp_channel] - last_timestamp) * 1000; // in ms
            if (time_wait - accumulated_time_delta > 1)
            {
#ifdef _WIN32
                Sleep ((int)(time_wait - accumulated_time_delta));
#else
                usleep ((int)(1000 * (time_wait - accumulated_time_delta)));
#endif
            }
            auto stop = std::chrono::high_resolution_clock::now ();
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count ();
            accumulated_time_delta += (duration / 1000.0 - time_wait);
        }

        last_timestamp = package[timestamp_channel];

        if (new_timestamps)
        {
            package[timestamp_channel] = get_timestamp ();
        }
        push_package (package, preset);
    }
    fclose (fp);
    delete[] package;
}

int PlaybackFileBoard::config_board (std::string config, std::string &response)
{
    if (strcmp (config.c_str (), SET_LOOPBACK_TRUE) == 0)
    {
        loopback = true;
    }
    else if (strcmp (config.c_str (), SET_LOOPBACK_FALSE) == 0)
    {
        loopback = false;
    }
    else if (strcmp (config.c_str (), NEW_TIMESTAMPS) == 0)
    {
        use_new_timestamps = true;
    }
    else if (strcmp (config.c_str (), OLD_TIMESTAMPS) == 0)
    {
        use_new_timestamps = false;
    }
    else if (strncmp (config.c_str (), SET_INDEX_PREFIX, strlen (SET_INDEX_PREFIX)) == 0)
    {
        try
        {
            double new_index = std::stod (config.substr (strlen (SET_INDEX_PREFIX)));
            if (((int)new_index >= 0) && ((int)new_index < 100))
            {
                lock.lock ();
                std::fill (pos_percentage.begin (), pos_percentage.end (), new_index);
                lock.unlock ();
            }
            else
            {
                safe_logger (
                    spdlog::level::err, "invalid index value, should be between 0 and 100");
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
        }
        catch (const std::exception &e)
        {
            safe_logger (spdlog::level::err, "need to write a number after {}, exception is: {}",
                SET_INDEX_PREFIX, e.what ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    else
    {
        safe_logger (spdlog::level::warn, "invalid config string {}", config);
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int PlaybackFileBoard::get_file_offsets (std::string filename, std::vector<long int> &offsets)
{
    offsets.clear ();

    FILE *fp = fopen (filename.c_str (), "rb");
    if (fp == NULL)
    {
        safe_logger (spdlog::level::err, "failed to open file: {}", filename.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    char buf[MAX_LINE_LENGTH];
    long int bytes_read = 0;
    while (true)
    {
        offsets.push_back (bytes_read);
        char *res = fgets (buf, sizeof (buf), fp);
        bytes_read += (long int)strlen (buf);
        if (res == NULL)
        {
            break;
        }
    }

    fclose (fp);
    if (offsets.size () < 2)
    {
        safe_logger (spdlog::level::err, "empty file: {}", filename);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}
