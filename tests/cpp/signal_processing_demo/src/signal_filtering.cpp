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
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        BrainFlowArray<double, 2> data = board->get_board_data ();
        board->release_session ();
        std::cout << "Original data:" << std::endl << data << std::endl;

        // apply filters
        int sampling_rate = BoardShim::get_sampling_rate ((int)BoardIds::SYNTHETIC_BOARD);
        std::vector<int> eeg_channels = BoardShim::get_eeg_channels (board_id);
        for (int i = 0; i < eeg_channels.size (); i++)
        {
            switch (i)
            {
                // just for test and demo - apply different filters to different eeg channels
                // signal filtering methods work in-place
                case 0:
                    DataFilter::perform_lowpass (data.get_address (eeg_channels[i]),
                        data.get_size (1), BoardShim::get_sampling_rate (board_id), 30.0, 3,
                        (int)FilterTypes::BUTTERWORTH, 0);
                    break;
                case 1:
                    DataFilter::perform_highpass (data.get_address (eeg_channels[i]),
                        data.get_size (1), BoardShim::get_sampling_rate (board_id), 5.0, 5,
                        (int)FilterTypes::CHEBYSHEV_TYPE_1, 1);
                    break;
                case 2:
                    DataFilter::perform_bandpass (data.get_address (eeg_channels[i]),
                        data.get_size (1), BoardShim::get_sampling_rate (board_id), 15.0, 10.0, 3,
                        (int)FilterTypes::BESSEL, 0);
                    break;
                case 3:
                    DataFilter::perform_bandstop (data.get_address (eeg_channels[i]),
                        data.get_size (1), BoardShim::get_sampling_rate (board_id), 50.0, 4.0, 4,
                        (int)FilterTypes::BUTTERWORTH, 0);
                    break;
                default:
                    DataFilter::remove_environmental_noise (data.get_address (eeg_channels[i]),
                        data.get_size (1), BoardShim::get_sampling_rate (board_id),
                        (int)NoiseTypes::FIFTY);
                    break;
            }
        }
        std::cout << "Filtered data:" << std::endl << data << std::endl;
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
