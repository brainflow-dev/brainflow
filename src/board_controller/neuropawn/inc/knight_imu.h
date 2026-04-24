#pragma once

#include "knight_base.h"

class KnightIMU : public KnightBase
{

protected:
    void read_thread () override;

public:
    KnightIMU (int board_id, struct BrainFlowInputParams params);
};
