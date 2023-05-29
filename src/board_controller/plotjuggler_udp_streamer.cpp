#include <cstdlib>
#include <sstream>
#include <string.h>
#include <string>

#include "board.h"
#include "brainflow_constants.h"
#include "brainflow_env_vars.h"
#include "plotjuggler_udp_streamer.h"


PlotJugglerUDPStreamer::PlotJugglerUDPStreamer (const char *ip, int port, json preset_descr)
    : Streamer ((int)preset_descr["num_rows"], "plotjuggler_udp", ip, std::to_string (port))
{
    strcpy (this->ip, ip);
    this->port = port;
    this->preset_descr = preset_descr;
    socket = NULL;
    is_streaming = false;
    db = NULL;
}

PlotJugglerUDPStreamer::~PlotJugglerUDPStreamer ()
{
    if ((streaming_thread.joinable ()) && (is_streaming))
    {
        is_streaming = false;
        streaming_thread.join ();
    }
    if (socket != NULL)
    {
        delete socket;
        socket = NULL;
    }
    if (db != NULL)
    {
        delete db;
        db = NULL;
    }
}

int PlotJugglerUDPStreamer::init_streamer ()
{
    if ((is_streaming) || (socket != NULL) || (db != NULL))
    {
        Board::board_logger->error ("plotjuggler streamer is running");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    socket = new SocketClientUDP (ip, port);
    int res = socket->connect ();
    if (res != (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        delete socket;
        socket = NULL;
        Board::board_logger->error ("failed to init udp socket {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    db = new DataBuffer (len, 1000);
    if (!db->is_ready ())
    {
        Board::board_logger->error ("unable to prepare buffer for streaming");
        delete db;
        db = NULL;
        delete socket;
        socket = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    is_streaming = true;
    streaming_thread = std::thread ([this] { this->thread_worker (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void PlotJugglerUDPStreamer::stream_data (double *data)
{
    db->add_data (data);
}

void PlotJugglerUDPStreamer::thread_worker ()
{
    double *transaction = new double[len];
    for (int i = 0; i < len; i++)
    {
        transaction[i] = 0.0;
    }
    std::string name = preset_descr["name"];
    while (is_streaming)
    {
        if (db->get_data_count () >= 1)
        {
            db->get_data (1, transaction);
            json j;
            j[name] = json::object ();
            for (auto &el : preset_descr.items ())
            {
                std::string key = el.key ();
                if (key.find ("_channels") != std::string::npos)
                {
                    std::string prefix = remove_substr (key, "_channels");
                    j[name][prefix] = json::object ();
                    std::vector<int> values = el.value ();
                    for (int i = 0; i < (int)values.size (); i++)
                    {
                        std::string channel_name = "channel " + std::to_string (i);
                        if ((key == "accel_channels") && (i == 0))
                            channel_name = "accel X";
                        if ((key == "accel_channels") && (i == 1))
                            channel_name = "accel Y";
                        if ((key == "accel_channels") && (i == 2))
                            channel_name = "accel Z";
                        if (key == "eeg_channels")
                        {
                            try
                            {
                                std::string eeg_names = preset_descr["eeg_names"];
                                std::vector<std::string> names_vec;
                                std::stringstream ss (eeg_names);
                                while (ss.good ())
                                {
                                    std::string substr;
                                    std::getline (ss, substr, ',');
                                    names_vec.push_back (substr);
                                }
                                channel_name = names_vec[i];
                            }
                            catch (...)
                            {
                            }
                        }
                        if ((values[i] >= 0) && (values[i] <= len))
                        {
                            j[name][prefix][channel_name] = transaction[values[i]];
                        }
                    }
                }
                else if (key.find ("_channel") != std::string::npos)
                {
                    int pos = el.value ();
                    std::string prefix = remove_substr (key, "_channel");
                    if ((pos >= 0) && (pos < len))
                    {
                        j[name][prefix] = transaction[pos];
                    }
                }
            }
            std::string s = j.dump ();
            socket->send (s.c_str (), (int)s.size ());
        }
        else
        {
#ifdef _WIN32
            Sleep (1);
#else
            usleep (10);
#endif
        }
    }
    delete[] transaction;
}

std::string PlotJugglerUDPStreamer::remove_substr (std::string str, std::string substr)
{
    std::string res = str;
    size_t pos = str.find (substr);
    if (pos != std::string::npos)
    {
        res.erase (pos, substr.length ());
    }
    return res;
}