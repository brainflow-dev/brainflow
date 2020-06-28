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

void print_head (double **data_buf, int num_channels, int num_data_points);

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

    try
    {
        board->prepare_session ();
        board->start_stream ();
        BoardShim::log_message ((int)LogLevels::LEVEL_INFO, "Start sleeping in the main thread");
#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        int data_count = 0;
        data = board->get_board_data (&data_count);
        BoardShim::log_message ((int)LogLevels::LEVEL_INFO, "read %d packages", data_count);
        board->release_session ();
        num_rows = BoardShim::get_num_rows (board_id);
        std::cout << std::endl << "Data from the board" << std::endl << std::endl;
        print_head (data, num_rows, data_count);

        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        for (int i = 0; i < eeg_num_channels; i++)
        {
            switch (i)
            {
                // for demo apply different methods to different channels, in production you should
                // choose one
                // first of all you can try simple moving average or moving median to remove noise
                case 0:
                    DataFilter::perform_rolling_filter (
                        data[eeg_channels[i]], data_count, 3, (int)AggOperations::MEDIAN);
                    break;
                case 1:
                    DataFilter::perform_rolling_filter (
                        data[eeg_channels[i]], data_count, 3, (int)AggOperations::MEAN);
                    break;
                case 2:
                    DataFilter::perform_rolling_filter (
                        data[eeg_channels[i]], data_count, 5, (int)AggOperations::MEDIAN);
                    break;
                case 3:
                    DataFilter::perform_rolling_filter (
                        data[eeg_channels[i]], data_count, 5, (int)AggOperations::MEAN);
                    break;
                    // if moving average and moving median dont work well for your signal you can
                    // try wavelet based denoising, feel free to try different wavelet functions and
                    // decomposition levels
                case 4:
                    DataFilter::perform_wavelet_denoising (
                        data[eeg_channels[i]], data_count, "db4", 3);
                    break;
                case 5:
                    DataFilter::perform_wavelet_denoising (
                        data[eeg_channels[i]], data_count, "coif3", 3);
                    break;
            }
        }
        std::cout << std::endl << "Data after denoising" << std::endl << std::endl;
        print_head (data, num_rows, data_count);
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

void print_head (double **data_buf, int num_channels, int num_data_points)
{
    std::cout << "Total Channels for this board: " << num_channels << std::endl;
    int num_points = (num_data_points < 5) ? num_data_points : 5;
    for (int i = 0; i < num_channels; i++)
    {
        std::cout << "Channel " << i << ": ";
        for (int j = 0; j < num_points; j++)
        {
            std::cout << data_buf[i][j] << ",";
        }
        std::cout << std::endl;
    }
}
