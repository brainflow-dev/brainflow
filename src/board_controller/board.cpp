#include <algorithm>
#include <string>
#include <vector>

#include "board.h"
#include "board_controller.h"
#include "custom_cast.h"
#include "file_streamer.h"
#include "multicast_streamer.h"
#include "plotjuggler_udp_streamer.h"

#include "spdlog/sinks/null_sink.h"

#define LOGGER_NAME "board_logger"

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
std::shared_ptr<spdlog::logger> Board::board_logger =
    spdlog::android_logger (LOGGER_NAME, "brainflow_ndk_logger");
#else
std::shared_ptr<spdlog::logger> Board::board_logger = spdlog::stderr_logger_mt (LOGGER_NAME);
#endif

JNIEnv *Board::java_jnienv = nullptr;

int Board::set_log_level (int level)
{
    int log_level = level;
    if (level > 6)
    {
        log_level = 6;
    }
    if (level < 0)
    {
        log_level = 0;
    }
    try
    {
        Board::board_logger->set_level (spdlog::level::level_enum (log_level));
        Board::board_logger->flush_on (spdlog::level::level_enum (log_level));
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::set_log_file (const char *log_file)
{
#ifdef __ANDROID__
    Board::board_logger->error ("For Android set_log_file is unavailable");
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
#else
    try
    {
        spdlog::level::level_enum level = Board::board_logger->level ();
        Board::board_logger = spdlog::create<spdlog::sinks::null_sink_st> (
            "null_logger"); // to dont set logger to nullptr and avoid race condition
        spdlog::drop (LOGGER_NAME);
        Board::board_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
        Board::board_logger->set_level (level);
        Board::board_logger->flush_on (level);
        spdlog::drop ("null_logger");
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int Board::prepare_for_acquisition (int buffer_size, const char *streamer_params)
{
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    for (auto it = dbs.begin (), next_it = it; it != dbs.end (); it = next_it)
    {
        ++next_it;
        delete it->second;
        dbs.erase (it);
    }
    for (auto it = marker_queues.begin (), next_it = it; it != marker_queues.end (); it = next_it)
    {
        ++next_it;
        it->second.clear ();
        marker_queues.erase (it);
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;

    std::vector<std::string> required_fields {
        "num_rows", "timestamp_channel", "name", "marker_channel"};
    std::vector<std::string> supported_presets {"ancillary", "auxiliary", "default"};
    for (std::string field : required_fields)
    {
        for (auto &el : board_descr.items ())
        {
            json board_preset = el.value ();
            std::string key = el.key ();
            if (std::find (supported_presets.begin (), supported_presets.end (), key) ==
                supported_presets.end ())
            {
                safe_logger (spdlog::level::err, "Preset {} is not supported", key);
                return (int)BrainFlowExitCodes::GENERAL_ERROR;
            }

            if (board_preset.find (field) == board_preset.end ())
            {
                safe_logger (spdlog::level::err,
                    "Field {} is not found in brainflow_boards.h for id {}", field, board_id);
                return (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
        }
    }

    if ((streamer_params != NULL) && (streamer_params[0] != '\0'))
    {
        res = add_streamer (streamer_params, (int)BrainFlowPresets::DEFAULT_PRESET);
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        for (auto &el : board_descr.items ())
        {
            json board_preset = el.value ();
            DataBuffer *db = new DataBuffer ((int)board_preset["num_rows"], buffer_size);
            if (!db->is_ready ())
            {
                safe_logger (
                    spdlog::level::err, "unable to prepare buffer with size {}", buffer_size);
                delete db;
                db = NULL;
                res = (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
            }
            else
            {
                int preset_int = preset_to_int (el.key ());
                dbs[preset_int] = db;
                marker_queues[preset_int] = std::deque<double> ();
            }
        }
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        free_packages ();
    }

    return res;
}

void Board::push_package (double *package, int preset)
{
    std::string preset_str = preset_to_string (preset);
    if ((board_descr.find (preset_str) == board_descr.end ()) || (dbs.find (preset) == dbs.end ()))
    {
        safe_logger (spdlog::level::err, "invalid json or push_package args, no such key");
        return;
    }

    lock.lock ();
    json board_preset = board_descr[preset_str];
    try
    {
        int marker_channel = board_preset["marker_channel"];
        if (marker_queues[preset].empty ())
        {
            package[marker_channel] = 0.0;
        }
        else
        {
            double marker = marker_queues[preset].at (0);
            package[marker_channel] = marker;
            marker_queues[preset].pop_front ();
        }
    }
    catch (...)
    {
        safe_logger (spdlog::level::err, "Failed to get marker channel/value");
    }

    if (dbs[preset] != NULL)
    {
        dbs[preset]->add_data (package);
    }
    if (streamers.find (preset) != streamers.end ())
    {
        for (auto &streamer : streamers[preset])
        {
            streamer->stream_data (package);
        }
    }
    lock.unlock ();
}

int Board::insert_marker (double value, int preset)
{
    if (std::fabs (value) < std::numeric_limits<double>::epsilon ())
    {
        safe_logger (spdlog::level::err, "0 is a default value for marker, you can not use it.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    std::string preset_str = preset_to_string (preset);
    if ((board_descr.find (preset_str) == board_descr.end ()) ||
        (marker_queues.find (preset) == marker_queues.end ()))
    {
        safe_logger (spdlog::level::err, "invalid preset");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    lock.lock ();
    marker_queues[preset].push_back (value);
    lock.unlock ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Board::free_packages ()
{
    for (auto it = dbs.begin (), next_it = it; it != dbs.end (); it = next_it)
    {
        ++next_it;
        delete it->second;
        dbs.erase (it);
    }

    for (auto it = marker_queues.begin (), next_it = it; it != marker_queues.end (); it = next_it)
    {
        ++next_it;
        it->second.clear ();
        marker_queues.erase (it);
    }

    for (auto it = streamers.begin (), next_it = it; it != streamers.end (); it = next_it)
    {
        ++next_it;
        for (auto &streamer : it->second)
        {
            delete streamer;
        }
        streamers.erase (it);
    }
}

int Board::add_streamer (const char *streamer_params, int preset)
{
    std::string preset_str = preset_to_string (preset);
    if (board_descr.find (preset_str) == board_descr.end ())
    {
        safe_logger (spdlog::level::err, "invalid preset");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int num_rows = (int)board_descr[preset_str]["num_rows"];
    std::string streamer_type = "";
    std::string streamer_dest = "";
    std::string streamer_mods = "";
    int res = parse_streamer_params (streamer_params, streamer_type, streamer_dest, streamer_mods);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    Streamer *streamer = NULL;

    if (streamer_type == "file")
    {
        safe_logger (spdlog::level::trace, "File Streamer, file: {}, mods: {}",
            streamer_dest.c_str (), streamer_mods.c_str ());
        streamer = new FileStreamer (streamer_dest.c_str (), streamer_mods.c_str (), num_rows);
    }
    if (streamer_type == "streaming_board")
    {
        int port = 0;
        try
        {
            port = std::stoi (streamer_mods);
        }
        catch (const std::exception &e)
        {
            safe_logger (spdlog::level::err, e.what ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        safe_logger (spdlog::level::trace, "MultiCast Streamer, ip addr: {}, port: {}",
            streamer_dest.c_str (), streamer_mods.c_str ());
        streamer = new MultiCastStreamer (streamer_dest.c_str (), port, num_rows);
    }
    if (streamer_type == "plotjuggler_udp")
    {
        int port = 0;
        try
        {
            port = std::stoi (streamer_mods);
        }
        catch (const std::exception &e)
        {
            safe_logger (spdlog::level::err, e.what ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        safe_logger (spdlog::level::trace, "PlotJuggler UDP Streamer, ip addr: {}, port: {}",
            streamer_dest.c_str (), streamer_mods.c_str ());
        streamer =
            new PlotJugglerUDPStreamer (streamer_dest.c_str (), port, board_descr[preset_str]);
    }

    if (streamer == NULL)
    {
        safe_logger (spdlog::level::err, "unsupported streamer type {}", streamer_type.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    res = streamer->init_streamer ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init streamer");
        delete streamer;
        streamer = NULL;
    }
    else
    {
        lock.lock ();
        streamers[preset].push_back (streamer);
        lock.unlock ();
    }

    return res;
}

int Board::delete_streamer (const char *streamer_params, int preset)
{
    if (streamers.find (preset) == streamers.end ())
    {
        safe_logger (spdlog::level::err, "no such streaming preset");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    std::string streamer_type = "";
    std::string streamer_dest = "";
    std::string streamer_mods = "";
    int res = parse_streamer_params (streamer_params, streamer_type, streamer_dest, streamer_mods);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    std::vector<Streamer *>::iterator it = streamers[preset].begin ();
    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    while (it != streamers[preset].end ())
    {
        if ((*it)->check_equals (streamer_type, streamer_dest, streamer_mods))
        {
            lock.lock ();
            delete *it;
            it = streamers[preset].erase (it);
            lock.unlock ();
            res = (int)BrainFlowExitCodes::STATUS_OK;
            safe_logger (spdlog::level::info, "streamer {} removed", streamer_params);
            break;
        }
        else
        {
            it++;
        }
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "no such streamer found");
    }
    return res;
}

int Board::parse_streamer_params (const char *streamer_params, std::string &streamer_type,
    std::string &streamer_dest, std::string &streamer_mods)
{
    if ((streamer_params == NULL) || (streamer_params[0] == '\0'))
    {
        safe_logger (spdlog::level::err, "invalid streamer params");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // parse string, sscanf doesnt work
    std::string streamer_params_str = streamer_params;
    size_t idx1 = streamer_params_str.find ("://");
    if (idx1 == std::string::npos)
    {
        safe_logger (spdlog::level::err, "format is streamer_type://streamer_dest:streamer_args");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    size_t idx2 = streamer_params_str.find_last_of (":", std::string::npos);
    if ((idx2 == std::string::npos) || (idx1 == idx2))
    {
        safe_logger (spdlog::level::err, "format is streamer_type://streamer_dest:streamer_args");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    streamer_type = streamer_params_str.substr (0, idx1);
    streamer_dest = streamer_params_str.substr (idx1 + 3, idx2 - idx1 - 3);
    streamer_mods = streamer_params_str.substr (idx2 + 1);

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::get_current_board_data (
    int num_samples, int preset, double *data_buf, int *returned_samples)
{
    std::string preset_str = preset_to_string (preset);
    if (board_descr.find (preset_str) == board_descr.end ())
    {
        safe_logger (spdlog::level::err, "invalid preset");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (dbs.find (preset) == dbs.end ())
    {
        safe_logger (spdlog::level::err,
            "stream is not started or no preset: {} found for this board", preset_str.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (!dbs[preset])
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if ((!data_buf) || (!returned_samples))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int num_rows = (int)board_descr[preset_str]["num_rows"];

    double *buf = new double[num_samples * num_rows];
    int num_data_points = (int)dbs[preset]->get_current_data (num_samples, buf);
    reshape_data (num_data_points, preset, buf, data_buf);
    delete[] buf;
    *returned_samples = num_data_points;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::get_board_data_count (int preset, int *result)
{
    if (dbs.find (preset) == dbs.end ())
    {
        safe_logger (spdlog::level::err,
            "stream is not startted or no preset: {} found for this board", preset);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (!dbs[preset])
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if (!result)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    *result = (int)dbs[preset]->get_data_count ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Board::get_board_data (int data_count, int preset, double *data_buf)
{
    std::string preset_str = preset_to_string (preset);
    if (board_descr.find (preset_str) == board_descr.end ())
    {
        safe_logger (spdlog::level::err, "invalid preset");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (dbs.find (preset) == dbs.end ())
    {
        safe_logger (spdlog::level::err,
            "stream is not started or no preset: {} found for this board", preset);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (!dbs[preset])
    {
        return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
    }
    if (!data_buf)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int num_rows = (int)board_descr[preset_str]["num_rows"];
    double *buf = new double[data_count * num_rows];
    int num_data_points = (int)dbs[preset]->get_data (data_count, buf);
    reshape_data (num_data_points, preset, buf, data_buf);
    delete[] buf;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Board::reshape_data (int data_count, int preset, const double *buf, double *output_buf)
{
    std::string preset_str = preset_to_string (preset);
    int num_rows = (int)board_descr[preset_str]["num_rows"];

    for (int i = 0; i < data_count; i++)
    {
        for (int j = 0; j < num_rows; j++)
        {
            output_buf[j * data_count + i] = buf[i * num_rows + j];
        }
    }
}

std::string Board::preset_to_string (int preset)
{
    if (preset == (int)BrainFlowPresets::DEFAULT_PRESET)
    {
        return "default";
    }
    else if (preset == (int)BrainFlowPresets::AUXILIARY_PRESET)
    {
        return "auxiliary";
    }
    else if (preset == (int)BrainFlowPresets::ANCILLARY_PRESET)
    {
        return "ancillary";
    }

    return "";
}

int Board::preset_to_int (std::string preset)
{
    if (preset == "default")
    {
        return (int)BrainFlowPresets::DEFAULT_PRESET;
    }
    else if (preset == "auxiliary")
    {
        return (int)BrainFlowPresets::AUXILIARY_PRESET;
    }
    else if (preset == "ancillary")
    {
        return (int)BrainFlowPresets::ANCILLARY_PRESET;
    }

    return (int)BrainFlowPresets::DEFAULT_PRESET;
}
