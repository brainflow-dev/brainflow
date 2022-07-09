#include <iostream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"
#include "data_filter.h"
#include "ml_model.h"

using namespace std;


int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();

    struct BrainFlowInputParams params;
    int board_id = (int)BoardIds::MUSE_S_BOARD;
    int res = 0;

    BoardShim *board = new BoardShim (board_id, params);

    try
    {
        board->prepare_session ();
        board->config_board ("p61");
        board->add_streamer ("file://streamer_default.csv:w");
        board->add_streamer ("file://streamer_aux.csv:w", (int)BrainFlowPresets::AUXILIARY_PRESET);
        board->add_streamer ("file://streamer_anc.csv:w", (int)BrainFlowPresets::ANCILLARY_PRESET);
        board->start_stream ();

#ifdef _WIN32
        Sleep (10000);
#else
        sleep (10);
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
