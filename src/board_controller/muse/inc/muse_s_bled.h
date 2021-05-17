#pragma once

#include "dyn_lib_board.h"


class MuseSBLED : public DynLibBoard<15>
{

private:
    static int num_objects;
    bool is_valid;
    bool use_mac_addr;

public:
    MuseSBLED (struct BrainFlowInputParams params);
    ~MuseSBLED ();

    int prepare_session ();

protected:
    std::string get_lib_name ();
    virtual int call_init ();
    virtual int call_open ();
};
