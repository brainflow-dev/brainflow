#include <fstream>
#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_controller.h"
#include "board_shim.h"
#include "data_handler.h"

using namespace std;


void write_csv (const char *filename, double **data_buf, int data_count, int total_channels)
{
    ofstream output_file;
    output_file.open (filename);
    for (int i = 0; i < data_count; i++)
    {
        for (int j = 0; j < total_channels; j++)
        {
            output_file << data_buf[i][j] << ",";
        }
        output_file << endl;
    }
    output_file.close ();
}

int main (int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "board id and port name are required" << endl;
        return -1;
    }

    set_log_level (0);

    int board_id = atoi (argv[1]);
    BoardShim *board = new BoardShim (board_id, argv[2]);
    int length = BoardInfoGetter::get_package_length (board_id);
    DataHandler *dh = new DataHandler (board_id);
    int buffer_size = 250 * 60;
    double **data_buf = new double *[buffer_size];
    for (int i = 0; i < buffer_size; i++)
    {
        data_buf[i] = new double[length];
    }
    int res = STATUS_OK;
    int data_count;

    try
    {
        std::cout << "preparing session" << std::endl;
        board->prepare_session ();
        std::cout << "starting streaming" << std::endl;
        board->start_stream (buffer_size);

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif
        std::cout << "stopping streaming" << std::endl;
        board->stop_stream ();
        std::cout << "getting data count" << std::endl;
        board->get_board_data_count (&data_count);
        std::cout << "getting data" << std::endl;
        board->get_board_data (data_count, data_buf);
        std::cout << "releasing session" << std::endl;
        board->release_session ();
        std::cout << "preprocessing data" << std::endl;
        dh->preprocess_data (data_buf, data_count);
    }
    catch (const BrainFlowException &err)
    {
        std::cout << err.what () << std::endl;
        res = err.get_exit_code ();
    }

    std::cout << "saving results" << std::endl;
    write_csv ("board_data.csv", data_buf, data_count, length);
    std::cout << "completed!" << std::endl;

    for (int i = 0; i < buffer_size; i++)
        delete[] data_buf[i];
    delete[] data_buf;

    delete dh;
    delete board;

    return res;
}