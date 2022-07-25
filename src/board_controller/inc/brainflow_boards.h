#pragma once

#include "json.hpp"

using json = nlohmann::json;

struct BrainFlowBoards
{
    json brainflow_boards_json;
    BrainFlowBoards ();
};

extern BrainFlowBoards boards_struct;
