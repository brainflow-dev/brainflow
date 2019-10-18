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
bool parse_args (int argc, char *argv[], struct BrainFlowInputParams *params, int *board_id);

int main (int argc, char *argv[])
{
    struct BrainFlowInputParams params;
    int board_id = 0;
    if (!parse_args (argc, argv, &params, &board_id))
    {
        return -1;
    }

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

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif
        board->stop_stream ();
        int data_count = 0;
        data = board->get_board_data (&data_count);
        std::cout << "received " << data_count << " packages" << std::endl;
        board->release_session ();
        num_rows = BoardShim::get_num_rows (board_id);
        print_head (data, num_rows, data_count);
        int eeg_num_channels = 0;
        eeg_channels = BoardShim::get_eeg_channels (board_id, &eeg_num_channels);
        print_head (data, num_rows, data_count);

        // just for test and demo - apply different filters to different eeg channels
        for (int i = 0; i < eeg_num_channels; i++)
        {
            switch (i)
            {
                case 0:
                    DataFilter::perform_lowpass (data[eeg_channels[i]], data_count,
                        BoardShim::get_sampling_rate (board_id), 20.0, 3, BUTTERWORTH, 0);
                    break;
                case 1:
                    DataFilter::perform_highpass (data[eeg_channels[i]], data_count,
                        BoardShim::get_sampling_rate (board_id), 1.0, 5, CHEBYSHEV_TYPE_1, 1);
                    break;
                case 2:
                    DataFilter::perform_bandpass (data[eeg_channels[i]], data_count,
                        BoardShim::get_sampling_rate (board_id), 15.0, 5.0, 3, BESSEL, 0);
                    break;
                case 3:
                    DataFilter::perform_bandstop (data[eeg_channels[i]], data_count,
                        BoardShim::get_sampling_rate (board_id), 5.0, 1.5, 4, BUTTERWORTH, 0);
                    break;
            }
        }
        print_head (data, num_rows, data_count);
    }
    catch (const BrainFlowException &err)
    {
        std::cout << err.what () << std::endl;
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

bool parse_args (int argc, char *argv[], struct BrainFlowInputParams *params, int *board_id)
{
    bool board_id_found = false;
    for (int i = 1; i < argc; i++)
    {
        if (std::string (argv[i]) == std::string ("--board-id"))
        {
            if (i + 1 < argc)
            {
                i++;
                board_id_found = true;
                *board_id = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--ip-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_address = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--ip-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_port = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--serial-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_port = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--ip-protocol"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_protocol = std::stoi (std::string (argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--other-info"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->other_info = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string (argv[i]) == std::string ("--mac-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->mac_address = std::string (argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
    }
    if (!board_id_found)
    {
        std::cerr << "board id is not provided" << std::endl;
        return false;
    }
    return true;
}