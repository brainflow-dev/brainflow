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

void print_one_row (double *data, int num_data_points);


int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();

    struct BrainFlowInputParams params;
    int res = 0;
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;
    // use synthetic board for demo
    BoardShim *board = new BoardShim (board_id, params);
    double *peaks = NULL;

    try
    {
        board->prepare_session ();
        board->start_stream ();

#ifdef _WIN32
        Sleep (15000);
#else
        sleep (15);
#endif

        board->stop_stream ();
        BrainFlowArray<double, 2> data = board->get_board_data ();
        board->release_session ();

        peaks = new double[data.get_size (1)];
        std::vector<int> eeg_channels = BoardShim::get_eeg_channels (board_id);

        for (int i = 0; i < eeg_channels.size (); i++)
        {
            DataFilter::restore_data_from_wavelet_detailed_coeffs (
                data.get_address (eeg_channels[i]), data.get_size (1), (int)WaveletTypes::DB4, 6, 4,
                peaks);
            DataFilter::detect_peaks_z_score (peaks, data.get_size (1), 20, 3.5, 0.0, peaks);
        }
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

    if (peaks != NULL)
        delete[] peaks;
    delete board;

    return res;
}
