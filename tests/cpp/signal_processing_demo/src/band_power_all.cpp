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
        BrainFlowArray<double, 2> data = board->get_board_data ();
        board->release_session ();
        std::cout << "Original data:" << std::endl << data << std::endl;

        // calc band powers
        int sampling_rate = BoardShim::get_sampling_rate ((int)BoardIds::SYNTHETIC_BOARD);
        std::vector<int> eeg_channels = BoardShim::get_eeg_channels (board_id);
        std::pair<double *, double *> bands =
            DataFilter::get_avg_band_powers (data, eeg_channels, sampling_rate, true);

        for (int i = 0; i < 5; i++)
        {
            // for synthetic board all channels have different freqs, relative stddev should be huge
            std::cout << "avg: " << bands.first[i] << " stddev: " << bands.second[i] << std::endl;
        }

        delete[] bands.first;
        delete[] bands.second;
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
