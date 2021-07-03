#pragma once

#include "dyn_lib_board.h"


class Muse2BLED : public DynLibBoard
{

private:
    static int num_objects;
    bool is_valid;

public:
    Muse2BLED (struct BrainFlowInputParams params);
    ~Muse2BLED ();

    int prepare_session ();

protected:
    std::string get_lib_name ();
};
