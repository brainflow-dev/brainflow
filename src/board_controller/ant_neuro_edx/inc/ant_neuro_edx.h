#pragma once

#include "dyn_lib_board.h"

class AntNeuroEdxBoard : public DynLibBoard
{
public:
    AntNeuroEdxBoard (int board_id, struct BrainFlowInputParams params);
    ~AntNeuroEdxBoard ();

protected:
    std::string get_lib_name () override;
};
