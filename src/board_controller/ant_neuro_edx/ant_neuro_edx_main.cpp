#include <memory>
#include <string>
#include <tuple>

#include "ant_neuro_edx_impl.h"
#include "board.h"
#include "shared_export.h"

std::unique_ptr<AntNeuroEdxInternalBoard> g_ant_edx_board = nullptr;

extern "C"
{
    SHARED_EXPORT int CALLING_CONVENTION initialize (void *param)
    {
        if (param == NULL)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        std::tuple<int, struct BrainFlowInputParams, json> *tuple =
            (std::tuple<int, struct BrainFlowInputParams, json> *)param;
        int board_id = std::get<0> (*tuple);
        struct BrainFlowInputParams params = std::get<1> (*tuple);
        g_ant_edx_board = std::unique_ptr<AntNeuroEdxInternalBoard> (
            new AntNeuroEdxInternalBoard (board_id, params));
        return g_ant_edx_board->prepare_session ();
    }

    SHARED_EXPORT int CALLING_CONVENTION open_device (void *param)
    {
        (void)param;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    SHARED_EXPORT int CALLING_CONVENTION start_stream (void *param)
    {
        (void)param;
        if (!g_ant_edx_board)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        return g_ant_edx_board->start_stream (128, "");
    }

    SHARED_EXPORT int CALLING_CONVENTION stop_stream (void *param)
    {
        (void)param;
        if (!g_ant_edx_board)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        return g_ant_edx_board->stop_stream ();
    }

    SHARED_EXPORT int CALLING_CONVENTION close_device (void *param)
    {
        (void)param;
        if (!g_ant_edx_board)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        return g_ant_edx_board->release_session ();
    }

    SHARED_EXPORT int CALLING_CONVENTION release (void *param)
    {
        (void)param;
        g_ant_edx_board = nullptr;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    SHARED_EXPORT int CALLING_CONVENTION config_device (void *param)
    {
        if (!g_ant_edx_board)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        if (param == NULL)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        std::string response;
        return g_ant_edx_board->config_board ((const char *)param, response);
    }

    SHARED_EXPORT int CALLING_CONVENTION get_data (void *param)
    {
        if (!g_ant_edx_board)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        if (param == NULL)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        int count = 0;
        int res =
            g_ant_edx_board->get_board_data_count ((int)BrainFlowPresets::DEFAULT_PRESET, &count);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        if (count <= 0)
        {
            return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
        }

        return g_ant_edx_board->get_board_data (
            1, (int)BrainFlowPresets::DEFAULT_PRESET, (double *)param);
    }
}
