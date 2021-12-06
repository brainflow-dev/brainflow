#pragma once

#include "dyn_lib_board.h"


class MuseBLED : public DynLibBoard
{

private:
    static int num_objects;
    bool is_valid;

public:
    MuseBLED (int board_id, struct BrainFlowInputParams params);
    ~MuseBLED ();

    int prepare_session ();

protected:
    std::string get_lib_name ();
};
