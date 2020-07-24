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
        Sleep (10000);
#else
        sleep (10);
#endif

        board->stop_stream ();
        int data_count = 0;
        int fft_len = DataFilter::get_nearest_power_of_two (sampling_rate);
        data = board->get_board_data (&data_count);
        board->release_session ();
        num_rows = BoardShim::get_num_rows (board_id);

        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        int filtered_size = 0;
        double *downsampled_data = NULL;
        // for synthetic board second channel is a sine wave at 10 Hz, should see big alpha
        int channel = eeg_channels[1];
        // optional - detrend
        DataFilter::detrend (data[channel], data_count, (int)DetrendOperations::LINEAR);
        std::pair<double *, double *> psd = DataFilter::get_psd_welch (data[channel], data_count,
            fft_len, fft_len / 2, sampling_rate, (int)WindowFunctions::HANNING);
        // calc band power
        double band_power_alpha = DataFilter::get_band_power (psd, fft_len / 2 + 1, 7.0, 13.0);
        double band_power_beta = DataFilter::get_band_power (psd, fft_len / 2 + 1, 14.0, 30.0);
        std::cout << "alpha/beta:" << band_power_alpha / band_power_beta << std::endl;
        // fail test if unexpected ratio
        if (band_power_alpha / band_power_beta < 100)
        {
            res = -1;
        }
        delete[] psd.first;
        delete[] psd.second;
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
