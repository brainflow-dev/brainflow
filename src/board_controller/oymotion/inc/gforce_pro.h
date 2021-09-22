#pragma once

#ifdef _WIN32
#include "dyn_lib_board.h"


class GforcePro : public DynLibBoard
{

private:
    static int num_objects;
    bool is_valid;

public:
    GforcePro (struct BrainFlowInputParams params);
    ~GforcePro ();

    int prepare_session ();

protected:
    std::string get_lib_name ();
};

#else

#include "board.h"


class GforcePro : public Board
{
public:
    GforcePro (struct BrainFlowInputParams params);
    ~GforcePro ();

    int prepare_session ();
    int start_stream (int buffer_size, const char *streamer_params);
    int stop_stream ();
    int release_session ();
    int config_board (std::string config, std::string &response);
};

#endif