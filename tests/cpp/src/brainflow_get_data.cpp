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

    int board_id = atoi (argv[1]);
    BoardShim *board = new BoardShim (board_id, argv[2]);
    BoardDesc *board_desc = board->board_desc;
    DataHandler *dh = new DataHandler (board_id);
    int buffer_size = 250 * 60;
    double **data_buf = new double *[buffer_size];
    for (int i = 0; i < buffer_size; i++)
    {
        data_buf[i] = new double[board_desc->get_total_count ()];
    }
    int res = STATUS_OK;
    int data_count;

    try
    {
        board->prepare_session ();
        board->start_stream (buffer_size);

#ifdef _WIN32
        Sleep (5000);
#else
        sleep (5);
#endif

        board->stop_stream ();
        board->get_board_data_count (&data_count);
        board->get_board_data (data_count, data_buf);
        board->release_session ();
        dh->preprocess_data (data_buf, data_count);
    }
    catch (const BrainFlowExcpetion &err)
    {
        std::cout << err.what () << std::endl;
        res = err.get_exit_code ();
    }

    write_csv ("board_data.csv", data_buf, data_count, board_desc->get_total_count ());

    for (int i = 0; i < buffer_size; i++)
        delete[] data_buf[i];
    delete[] data_buf;

    delete dh;
    delete board;

    return res;
}