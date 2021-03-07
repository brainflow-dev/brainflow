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
    // use synthetic board for demo
    BoardShim *board = new BoardShim ((int)BoardIds::SYNTHETIC_BOARD, params);

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
        BrainFlowArray<double, 2> data = board->get_current_board_data (128);
        board->release_session ();
        std::cout << "Original data:" << std::endl << data << std::endl;

        // apply filters
        int sampling_rate = BoardShim::get_sampling_rate ((int)BoardIds::SYNTHETIC_BOARD);
        std::vector<int> eeg_channels =
            BoardShim::get_eeg_channels ((int)BoardIds::SYNTHETIC_BOARD);
        int data_count = data.get_size (1);
        for (int i = 0; i < eeg_channels.size (); i++)
        {
            // demo for wavelet transform
            // std::pair of coeffs array in format[A(J) D(J) D(J-1) ..... D(1)] where J is a
            // decomposition level, A - app coeffs, D - detailed coeffs, and array which stores
            // length for each block, len of this array is decomposition_length + 1
            std::pair<double *, int *> wavelet_output = DataFilter::perform_wavelet_transform (
                data.get_address (eeg_channels[i]), data_count, "db4", 4);
            // you can do smth with wavelet coeffs here, for example denoising works via thresholds
            // for wavelet coefficients
            std::cout << "approximation coefficients:" << std::endl;
            for (int i = 0; i < wavelet_output.second[0]; i++)
            {
                std::cout << wavelet_output.first[i] << " ";
            }
            std::cout << std::endl;
            std::cout << "first block of detailed coefficients:" << std::endl;
            for (int i = wavelet_output.second[0];
                 i < wavelet_output.second[0] + wavelet_output.second[1]; i++)
            {
                std::cout << wavelet_output.first[i] << " ";
            }
            std::cout << std::endl;

            double *restored_data = DataFilter::perform_inverse_wavelet_transform (
                wavelet_output, data_count, "db4", 4);

            std::cout << "Original data:" << std::endl;
            print_one_row (data.get_address (eeg_channels[i]), data_count);
            std::cout << "Restored after inverse wavelet transform data:" << std::endl;
            print_one_row (restored_data, data_count);

            delete[] wavelet_output.first;
            delete[] restored_data;
            delete[] wavelet_output.second;

            // demo for fft
            // data count must be power of 2 for fft!
            std::complex<double> *fft_data = DataFilter::perform_fft (
                data.get_address (eeg_channels[i]), data_count, (int)WindowFunctions::NO_WINDOW);
            // len of fft_data array is N / 2 + 1
            std::cout << "FFT coeffs:" << std::endl;
            for (int i = 0; i < data_count / 2 + 1; i++)
            {
                std::cout << fft_data[i] << " ";
            }
            std::cout << std::endl;
            double *restored_from_fft_data = DataFilter::perform_ifft (fft_data, data_count);
            std::cout << "Restored after inverse fft transform data:" << std::endl;
            print_one_row (restored_from_fft_data, data_count);

            delete[] fft_data;
            delete[] restored_from_fft_data;
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

    delete board;

    return res;
}

void print_one_row (double *data, int num_data_points)
{
    for (int i = 0; i < num_data_points; i++)
    {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
}
