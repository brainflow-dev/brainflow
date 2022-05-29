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
        json board_descr = BoardShim::get_board_descr (board_id);
        int sampling_rate = (int)board_descr["sampling_rate"];
        int fft_len = DataFilter::get_nearest_power_of_two (sampling_rate);
        std::vector<int> eeg_channels = board_descr["eeg_channels"];
        // for synthetic board second channel is a sine wave at 10 Hz, should see big alpha
        int channel = eeg_channels[1];
        // optional - detrend
        DataFilter::detrend (
            data.get_address (channel), data.get_size (1), (int)DetrendOperations::LINEAR);
        std::cout << "Data after detrend:" << std::endl << data << std::endl;
        std::pair<double *, double *> psd = DataFilter::get_psd_welch (data.get_address (channel),
            data.get_size (1), fft_len, fft_len / 2, sampling_rate, (int)WindowFunctions::HANNING);
        // calc band power
        double band_power_alpha = DataFilter::get_band_power (psd, fft_len / 2 + 1, 7.0, 13.0);
        double band_power_beta = DataFilter::get_band_power (psd, fft_len / 2 + 1, 14.0, 30.0);
        std::cout << "alpha/beta:" << band_power_alpha / band_power_beta << std::endl;
        delete[] psd.first;
        delete[] psd.second;
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
