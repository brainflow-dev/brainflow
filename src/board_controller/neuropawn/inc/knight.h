#pragma once

#include "knight_base.h"

class Knight : public KnightBase
{

protected:
    void read_thread ();

public:
    Knight (int board_id, struct BrainFlowInputParams params);
};