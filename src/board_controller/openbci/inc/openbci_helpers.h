#pragma once

#include "board.h"
#include "board_controller.h"

inline int validate_config (char *config)
{
    if (!config)
    {
        Board::board_logger->error ("empty config message");
        return INVALID_ARGUMENTS_ERROR;
    }
    if ((config[0] == 'b') || (config[0] == 's'))
    {
        Board::board_logger->error ("Config message can not starts with 'b' or 's', to start and "
                                    "stop streaming you need to use dedicated methods");
        return INVALID_ARGUMENTS_ERROR;
    }
    return STATUS_OK;
}
