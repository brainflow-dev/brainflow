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
    ~BrainBitBLED () override;

    int prepare_session () override;

protected:
    std::string get_lib_name () override;
    int call_open () override;
};
