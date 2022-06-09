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

    try
    {
        board->prepare_session ();
        board->start_stream ();

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        BrainFlowArray<double, 2> data = board->get_board_data (32);
        board->release_session ();

        double *downsampled_data = NULL;
        int filtered_size = 0;
        std::vector<int> eeg_channels = BoardShim::get_eeg_channels (board_id);

        for (int i = 0; i < eeg_channels.size (); i++)
        {
            std::cout << "Data from :" << eeg_channels[i] << " before downsampling " << std::endl;
            print_one_row (data.get_address (eeg_channels[i]), data.get_size (1));

            // just for demo apply different downsampling algorithms to different channels
            // downsampling here just aggregates data points
            switch (i)
            {
                case 0:
                    downsampled_data =
                        DataFilter::perform_downsampling (data.get_address (eeg_channels[i]),
                            data.get_size (1), 2, (int)AggOperations::MEAN, &filtered_size);
                    break;
                case 1:
                    downsampled_data =
                        DataFilter::perform_downsampling (data.get_address (eeg_channels[i]),
                            data.get_size (1), 3, (int)AggOperations::MEDIAN, &filtered_size);
                    break;
                default:
                    downsampled_data =
                        DataFilter::perform_downsampling (data.get_address (eeg_channels[i]),
                            data.get_size (1), 2, (int)AggOperations::EACH, &filtered_size);
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
    // print only first 10 data points
    int num_points = (num_data_points < 10) ? num_data_points : 10;
    for (int i = 0; i < num_points; i++)
    {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
}
