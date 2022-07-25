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
    std::cout << "BoardShim Version: " << BoardShim::get_version () << std::endl;
    std::cout << "DataFilter Version: " << DataFilter::get_version () << std::endl;
    std::cout << "MLModel Version: " << MLModel::get_version () << std::endl;

    struct BrainFlowInputParams params;
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;
    int res = 0;

    BoardShim *board = new BoardShim (board_id, params);

    try
    {
        board->prepare_session ();
        board->add_streamer ("file://streamer_default.csv:w");
        board->start_stream ();
        board->add_streamer ("file://streamer_aux.csv:w", (int)BrainFlowPresets::AUXILIARY_PRESET);

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        BrainFlowArray<double, 2> data = board->get_board_data ();
        std::cout << data << std::endl;
        data = board->get_current_board_data (10);
        std::cout << data << std::endl;
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
