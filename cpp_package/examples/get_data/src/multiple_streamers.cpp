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



int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();
    std::cout << "BoardShim Version: " << BoardShim::get_version () << std::endl;
    std::cout << "DataFilter Version: " << DataFilter::get_version () << std::endl;
    std::cout << "MLModel Version: " << MLModel::get_version () << std::endl;

    struct BrainFlowInputParams params;
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;
    int res = 0;

    BoardShim *board_1 = new BoardShim (board_id, params);


    try
    {
        board_1->prepare_session ();
        board_1->add_streamer ("file://streamer_default.csv:w");
        board_1->add_streamer ("file://streamer_aux.csv:w");
        board_1->start_stream ();

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif
        
        board_1->stop_stream ();
        board_1->release_session ();
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
        if (board_1->is_prepared ())
        {
            board_1->release_session ();
        }
    }

    delete board_1;


    // adding the second streamer after starting stream.

    BoardShim *board_2 = new BoardShim (board_id, params);
    try
    {
        board_2->prepare_session ();
        board_2->add_streamer ("file://streamer_default.csv:w");
        board_2->start_stream ();
        board_2->add_streamer ("file://streamer_aux.csv:w");

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board_2->stop_stream ();
        board_2->release_session ();
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
        if (board_2->is_prepared ())
        {
            board_2->release_session ();
        }
    }

    delete board_2;

    return res;
}
