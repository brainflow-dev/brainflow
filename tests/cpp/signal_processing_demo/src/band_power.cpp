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

int main (int argc, char *argv[])
{
    struct BrainFlowInputParams params;
    // use synthetic board for demo
    int board_id = SYNTHETIC_BOARD;

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
        Sleep (10000);
#else
        sleep (10);
#endif

        board->stop_stream ();
        int data_count = 0;
        int fft_len = DataFilter::get_nearest_power_of_two (sampling_rate);
        data = board->get_current_board_data (fft_len, &data_count);
        if (data_count != fft_len)
        {
            BoardShim::log_message ((int)LogLevels::LEVEL_ERROR,
                "read %d packages, for this test we want exactly %d packages", data_count, fft_len);
            return GENERAL_ERROR;
        }
        board->release_session ();
        num_rows = BoardShim::get_num_rows (board_id);

        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        int filtered_size = 0;
        double *downsampled_data = NULL;
        for (int i = 0; i < eeg_num_channels; i++)
        {
            // optional: you can subtract mean from signal before FFT or apply filters
            // calc psd
            std::pair<double *, double *> psd =
                DataFilter::get_psd (data[eeg_channels[i]], data_count, sampling_rate, HAMMING);
            // calc band power
            double band_power_alpha =
                DataFilter::get_band_power (psd, data_count / 2 + 1, 7.0, 13.0);
            double band_power_beta =
                DataFilter::get_band_power (psd, data_count / 2 + 1, 14.0, 30.0);
            std::cout << "alpha/beta:" << band_power_alpha / band_power_beta << std::endl;
            delete[] psd.first;
            delete[] psd.second;
        }
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
