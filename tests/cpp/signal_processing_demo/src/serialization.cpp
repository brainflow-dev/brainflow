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
    int board_id = SYNTHETIC_BOARD;

    BoardShim::enable_dev_board_logger ();

    BoardShim *board = new BoardShim (board_id, params);
    double **data = NULL;
    int res = 0;
    int num_rows = 0;

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

        // demo for serialization
        DataFilter::write_file (
            data, num_rows, data_count, "test.csv", "w"); // use "a" for append mode
        int restored_num_rows = 0;
        int restored_num_cols = 0;
        double **restored_data =
            DataFilter::read_file (&restored_num_rows, &restored_num_cols, "test.csv");
        std::cout << std::endl
                  << "Data from the file, num packages is " << restored_num_cols << std::endl
                  << std::endl;
        print_head (restored_data, restored_num_rows, restored_num_cols);
        for (int i = 0; i < restored_num_rows; i++)
        {
            delete[] restored_data[i];
        }
        delete[] restored_data;
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
