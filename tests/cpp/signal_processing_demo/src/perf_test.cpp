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
        // sleep 100 seconds to collect more data
#ifdef _WIN32
        Sleep (100000);
#else
        sleep (100);
#endif

        board->stop_stream ();
        int data_count = 0;
        data = board->get_board_data (&data_count);
        board->release_session ();
        num_rows = BoardShim::get_num_rows (board_id);

        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        int filtered_size = 0;

        // run welch method and band powers
        // at this instant use function now()
        auto start = high_resolution_clock::now ();
        int fft_len = DataFilter::get_nearest_power_of_two (sampling_rate) * 2;
        int welch_channel = eeg_channels[1];
        DataFilter::detrend (data[welch_channel], data_count, (int)DetrendOperations::LINEAR);
        std::pair<double *, double *> psd = DataFilter::get_psd_welch (data[welch_channel],
            data_count, fft_len, fft_len / 2, sampling_rate, (int)WindowFunctions::HANNING);
        double band_power_alpha = DataFilter::get_band_power (psd, fft_len / 2 + 1, 7.0, 13.0);
        double band_power_beta = DataFilter::get_band_power (psd, fft_len / 2 + 1, 14.0, 30.0);
        auto stop = high_resolution_clock::now ();
        auto duration = duration_cast<microseconds> (stop - start);
        cout << "Welch + BandPowers : " << duration.count () << endl;

        // run filter with high order
        start = high_resolution_clock::now ();
        int filter_channel = eeg_channels[2];
        DataFilter::perform_bandpass (data[filter_channel], data_count, sampling_rate, 30.0, 30.0,
            7, (int)FilterTypes::BUTTERWORTH, 0);
        stop = high_resolution_clock::now ();
        duration = duration_cast<microseconds> (stop - start);
        cout << "Filter : " << duration.count () << endl;

        // run denoising
        start = high_resolution_clock::now ();
        int denoising_channel = eeg_channels[3];
        DataFilter::perform_wavelet_denoising (data[denoising_channel], data_count, "db4", 4);
        stop = high_resolution_clock::now ();
        duration = duration_cast<microseconds> (stop - start);
        cout << "Denoising : " << duration.count () << endl;

        // run rolling filter
        start = high_resolution_clock::now ();
        int rolling_filter_channel = eeg_channels[4];
        DataFilter::perform_rolling_filter (
            data[rolling_filter_channel], data_count, 3, (int)AggOperations::MEDIAN);
        stop = high_resolution_clock::now ();
        duration = duration_cast<microseconds> (stop - start);
        cout << "Moving Median : " << duration.count () << endl;

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
