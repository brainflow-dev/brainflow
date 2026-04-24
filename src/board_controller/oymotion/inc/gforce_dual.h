#pragma once

#ifdef _WIN32
#include "dyn_lib_board.h"


class GforceDual : public DynLibBoard
{

private:
    static int num_objects;
    bool is_valid;

public:
    GforceDual (struct BrainFlowInputParams params);
    ~GforceDual () override;

    int prepare_session () override;

protected:
    std::string get_lib_name () override;
};

#else

#include "board.h"


class GforceDual : public Board
{
public:
    GforceDual (struct BrainFlowInputParams params);
    ~GforceDual () override;

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};

#endif
