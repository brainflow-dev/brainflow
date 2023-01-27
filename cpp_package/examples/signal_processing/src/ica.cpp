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

using namespace std;


int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();

    struct BrainFlowInputParams params;
    int res = 0;
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;
    std::vector<int> eeg_channels = BoardShim::get_eeg_channels (board_id);
    int channel_to_use = eeg_channels[4];
    // use synthetic board for demo
    BoardShim *board = new BoardShim (board_id, params);

    try
    {
        board->prepare_session ();
        board->start_stream ();

#ifdef _WIN32
        Sleep (10000);
#else
        sleep (10);
#endif

        board->stop_stream ();
        BrainFlowArray<double, 2> data =
            board->get_board_data (500, (int)BrainFlowPresets::DEFAULT_PRESET);
        board->release_session ();

        BrainFlowArray<double, 2> data_reshaped (data.get_address (channel_to_use), 5, 100);
        std::tuple<BrainFlowArray<double, 2>, BrainFlowArray<double, 2>, BrainFlowArray<double, 2>,
            BrainFlowArray<double, 2>>
            returned_matrixes = DataFilter::perform_ica (data_reshaped, 2);
        std::cout << std::get<3> (returned_matrixes) << std::endl;
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
