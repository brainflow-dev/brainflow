#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"
#include "brainflow_filter.h"
#include "data_filter.h"

using namespace std;


int main (int argc, char *argv[])
{
    BoardShim::enable_dev_board_logger ();

    struct BrainFlowInputParams params;
    int res = 0;
    int board_id = (int)BoardIds::SYNTHETIC_BOARD;
    // use synthetic board for demo
    BoardShim board {board_id, params};

    try
    {
        board.prepare_session ();
        board.start_stream ();

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board.stop_stream ();
        BrainFlowArray<double, 2> data = board.get_board_data ();
        board.release_session ();
        std::cout << "Original data:" << std::endl << data << std::endl;

        // apply filters
        int sampling_rate = BoardShim::get_sampling_rate ((int)BoardIds::SYNTHETIC_BOARD);
        std::vector<int> eeg_channels = BoardShim::get_eeg_channels (board_id);
        const auto data_length = data.get_size (1);
        auto batch_data = std::make_unique<double[]> (data_length);
        auto state_data = std::make_unique<double[]> (data_length);

        std::random_device rd;
        std::uniform_int_distribution<size_t> draw_length {0, 1U + data_length / 5U};

        auto stateful_filtering = [&draw_length, &rd] (std::unique_ptr<brainflow_filter> &filter,
                                      double *data, const size_t data_length) {
            size_t current_idx = 0;
            while (current_idx < data_length)
            {
                const size_t batch_length = std::max (draw_length (rd), data_length - current_idx);
                filter->process (data + current_idx, static_cast<int> (batch_length));
                current_idx += batch_length;
            }
        };

        for (int i = 0; i < eeg_channels.size (); i++)
        {
            std::copy_n (data.get_address (eeg_channels[i]), data_length, batch_data.get ());
            std::copy_n (data.get_address (eeg_channels[i]), data_length, state_data.get ());
            BrainFlowArray<double, 2> bf_orig {batch_data.get (), 1, data_length};
            std::cout << "Original data:" << std::endl << bf_orig << std::endl;
            switch (i)
            {
                // just for test and demo - apply different filters to different eeg channels
                // signal filtering methods work in-place
                case 0:
                {
                    DataFilter::perform_lowpass (batch_data.get (), data_length,
                        BoardShim::get_sampling_rate (board_id), 30.0, 3,
                        (int)FilterTypes::BUTTERWORTH, 0);
                    std::unique_ptr<brainflow_filter> filter =
                        DataFilter::create_lowpass_filter (BoardShim::get_sampling_rate (board_id),
                            30.0, 3, (int)FilterTypes::BUTTERWORTH, 0);
                    stateful_filtering (filter, state_data.get (), data_length);
                }
                break;
                case 1:
                {
                    DataFilter::perform_highpass (batch_data.get (), data_length,
                        BoardShim::get_sampling_rate (board_id), 5.0, 5,
                        (int)FilterTypes::CHEBYSHEV_TYPE_1, 1);
                    std::unique_ptr<brainflow_filter> filter =
                        DataFilter::create_highpass_filter (BoardShim::get_sampling_rate (board_id),
                            5.0, 5, (int)FilterTypes::CHEBYSHEV_TYPE_1, 1);
                    stateful_filtering (filter, state_data.get (), data_length);
                }
                break;
                case 2:
                {
                    DataFilter::perform_bandpass (batch_data.get (), data_length,
                        BoardShim::get_sampling_rate (board_id), 15.0, 10.0, 3,
                        (int)FilterTypes::BESSEL, 0);
                    std::unique_ptr<brainflow_filter> filter =
                        DataFilter::create_bandpass_filter (BoardShim::get_sampling_rate (board_id),
                            15.0, 10.0, 3, (int)FilterTypes::BESSEL, 0);
                    stateful_filtering (filter, state_data.get (), data_length);
                }
                break;
                case 3:
                {
                    DataFilter::perform_bandstop (batch_data.get (), data_length,
                        BoardShim::get_sampling_rate (board_id), 50.0, 4.0, 4,
                        (int)FilterTypes::BUTTERWORTH, 0);
                    std::unique_ptr<brainflow_filter> filter =
                        DataFilter::create_bandstop_filter (BoardShim::get_sampling_rate (board_id),
                            50.0, 4.0, 4, (int)FilterTypes::BUTTERWORTH, 0);
                    stateful_filtering (filter, state_data.get (), data_length);
                }
                break;
                default:
                {
                    DataFilter::remove_environmental_noise (batch_data.get (), data_length,
                        BoardShim::get_sampling_rate (board_id), (int)NoiseTypes::FIFTY);
                    std::unique_ptr<brainflow_filter> filter =
                        DataFilter::create_environmental_noise_filter (
                            BoardShim::get_sampling_rate (board_id), (int)NoiseTypes::FIFTY);
                    stateful_filtering (filter, state_data.get (), data_length);
                }
                break;
            }
            BrainFlowArray<double, 2> bf_batch {batch_data.get (), 1, data_length};
            std::cout << "Batch data:" << std::endl << bf_batch << std::endl;
            double sum_abs_diff = 0.0;
            for (int j = 0; j < data_length; ++j)
            {
                const auto abs_diff = std::abs (batch_data[j] - state_data[j]);
                if (abs_diff > 1e-15)
                {
                    return 1;
                }
                sum_abs_diff += abs_diff;
            }
            std::cout << "abs diff sum:" << sum_abs_diff << std::endl;
        }
        std::cout << "Filtered data:" << std::endl << data << std::endl;
    }
    catch (const BrainFlowException &err)
    {
        BoardShim::log_message ((int)LogLevels::LEVEL_ERROR, err.what ());
        res = err.exit_code;
        if (board.is_prepared ())
        {
            board.release_session ();
        }
    }

    return res;
}
