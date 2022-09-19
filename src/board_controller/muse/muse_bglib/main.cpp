#include <stdlib.h>
#include <tuple>

#include "brainflow_constants.h"
#include "brainflow_input_params.h"

#include "json.hpp"
#include "muse_bglib_helper.h"
#include "muse_functions.h"
#include "uart.h"

using json = nlohmann::json;


MuseBGLibHelper *helper = NULL;


void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
{
    if (helper != NULL)
    {
        if (uart_tx (len1, data1) || uart_tx (len2, data2))
        {
        }
    }
}

int initialize (void *param)
{
    if (helper != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }
    std::tuple<int, struct BrainFlowInputParams, json> *info =
        (std::tuple<int, struct BrainFlowInputParams, json> *)param;
    helper = new MuseBGLibHelper (std::get<0> (*info), std::get<2> (*info));
    int res = helper->initialize (std::get<1> (*info));
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete helper;
        helper = NULL;
    }
    else
    {
        bglib_output = output;
    }
    return res;
}

int open_device (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->open_device ();
}

int stop_stream (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->stop_stream ();
}

int start_stream (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->start_stream ();
}

int close_device (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->close_device ();
}

int get_data_default (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->get_data_default (param);
}

int get_data_aux (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->get_data_aux (param);
}

int get_data_anc (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->get_data_anc (param);
}

int release (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = helper->release ();
    delete helper;
    helper = NULL;
    return res;
}

int config_device (void *param)
{
    if (helper == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    return helper->config_device ((const char *)param);
}
