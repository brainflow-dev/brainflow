#pragma once

#include "dyn_lib_board.h"


class BrainBitBLED : public DynLibBoard
{

private:
    static int num_objects;
    bool is_valid;
    bool use_mac_addr;

public:
    BrainBitBLED (struct BrainFlowInputParams params);
    ~BrainBitBLED ();

    int prepare_session ();

protected:
    std::string get_lib_name ();
    virtual int call_open ();
};
