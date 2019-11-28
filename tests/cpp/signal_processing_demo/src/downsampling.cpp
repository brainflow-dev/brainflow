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

void print_one_row (double *data, int num_data_points);

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

        // downsample only eeg channels and print them
        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        double *downsampled_data = NULL;
        int filtered_size = 0;
        for (int i = 0; i < eeg_num_channels; i++)
        {
            std::cout << "Data from :" << eeg_channels[i] << " before downsampling " << std::endl;
            print_one_row (data[eeg_channels[i]], data_count);

            // just for demo apply different downsampling algorithms to different channels
            // downsampling here doesnt apply lowpass filter for you, it just aggregates data points
            switch (i)
            {
                case 0:
                    downsampled_data = DataFilter::perform_downsampling (
                        data[eeg_channels[i]], data_count, 2, MEAN, &filtered_size);
                    break;
                case 1:
                    downsampled_data = DataFilter::perform_downsampling (
                        data[eeg_channels[i]], data_count, 3, MEDIAN, &filtered_size);
                    break;
                default:
                    downsampled_data = DataFilter::perform_downsampling (
                        data[eeg_channels[i]], data_count, 2, EACH, &filtered_size);
                    break;
            }

            std::cout << "Data from :" << eeg_channels[i] << " after downsampling " << std::endl;
            print_one_row (downsampled_data, filtered_size);
            delete[] downsampled_data;
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

void print_one_row (double *data, int num_data_points)
{
    // print only first 10 data points
    int num_points = (num_data_points < 10) ? num_data_points : 10;
    for (int i = 0; i < num_points; i++)
    {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
}