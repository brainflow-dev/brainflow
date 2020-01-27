#include <iostream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"

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
    int res = 0;
    int num_rows = 0;

    // Run two sessions one after another to test that release_session frees all resources
    for (int i = 0; i < 2; i++)
    {
        try
        {
            board->prepare_session ();
            board->start_stream ();
            BoardShim::log_message (
                (int)LogLevels::LEVEL_INFO, "Start sleeping in the main thread");
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
            // for STREAMING_BOARD you have to query information using board id for master board
            // because for STREAMING_BOARD data format is determined by master board!
            if (board_id == STREAMING_BOARD)
            {
                board_id = std::stoi (params.other_info);
                BoardShim::log_message ((int)LogLevels::LEVEL_INFO, "Use Board Id %d", board_id);
            }
            num_rows = BoardShim::get_num_rows (board_id);
            std::cout << std::endl << "Data from the board" << std::endl << std::endl;
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
    }

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