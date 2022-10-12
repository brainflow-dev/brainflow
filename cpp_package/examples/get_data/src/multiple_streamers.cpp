#include <iostream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"


int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();

    struct BrainFlowInputParams params;
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;
    int res = 0;

    BoardShim *board = new BoardShim (board_id, params);

    try
    {
        board->prepare_session ();
        board->add_streamer ("file://streamer_default_1.csv:w");
        board->add_streamer ("file://streamer_default_2.csv:w");
        board->add_streamer (
            "file://streamer_aux_1.csv:w", (int)BrainFlowPresets::AUXILIARY_PRESET);
        board->add_streamer (
            "file://streamer_aux_2.csv:w", (int)BrainFlowPresets::AUXILIARY_PRESET);
        board->start_stream ();

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif
        board->delete_streamer (
            "file://streamer_aux_2.csv:w", (int)BrainFlowPresets::AUXILIARY_PRESET);

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        board->release_session ();
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
        if (board->is_prepared ())
        {
            board->release_session ();
        }
    }

    delete board;

    return res;
}
