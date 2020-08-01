#include <chrono>
#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"
#include "data_filter.h"

using namespace std;
using namespace std::chrono;

int main (int argc, char *argv[])
{
    struct BrainFlowInputParams params;
    // use synthetic board for demo
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;

    BoardShim::enable_dev_board_logger ();

    BoardShim *board = new BoardShim (board_id, params);
    double **data = NULL;
    int *eeg_channels = NULL;
    int num_rows = 0;
    int res = 0;
    int sampling_rate = BoardShim::get_sampling_rate (board_id);

    try
    {
        board->prepare_session ();
        board->start_stream ();
        BoardShim::log_message ((int)LogLevels::LEVEL_INFO, "Start sleeping in the main thread");
#ifdef _WIN32
        Sleep (20000);
#else
        sleep (20);
#endif

        board->stop_stream ();
        int data_count = 0;
        data = board->get_board_data (&data_count);
        board->release_session ();
        num_rows = BoardShim::get_num_rows (board_id);

        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        auto start = high_resolution_clock::now ();
        std::pair<double *, double *> bands = DataFilter::get_avg_band_powers (
            data, data_count, eeg_channels, eeg_num_channels, sampling_rate, true);
        auto stop = high_resolution_clock::now ();
        auto duration = duration_cast<microseconds> (stop - start);
        // set OMP_NUM_THREADS env variable to control calc time
        std::cout << "Calc time : " << duration.count () << endl;
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
    }

    if (data != NULL)
    {
        for (int i = 0; i < num_rows; i++)
        {
            delete[] data[i];
        }
    }
    delete[] data;
    delete[] eeg_channels;
    delete board;

    return res;
}
