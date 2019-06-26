#include <stdlib.h>
#include <string.h>

#include "board_controller.h"
#include "board_shim.h"

BoardShim::BoardShim (int board_id, const char *port_name)
{
    strcpy (this->port_name, port_name);
    this->board_id = board_id;
    if (board_id == CYTON_BOARD)
    {
        num_data_channels = 12;                 // package_num, 8 eeg, 3accel
        total_channels = num_data_channels + 1; // plus ts
    }
    else
    {
        if (board_id == GANGLION_BOARD)
        {
            num_data_channels = 8;                  // package_num, 8 eeg, 3accel
            total_channels = num_data_channels + 1; // plus ts
        }
    }
}

int BoardShim::prepare_session ()
{
    return ::prepare_session (board_id, port_name);
}

int BoardShim::start_stream (int buffer_size)
{
    return ::start_stream (buffer_size);
}

int BoardShim::stop_stream ()
{
    return ::stop_stream ();
}

int BoardShim::release_session ()
{
    return ::release_session ();
}

int BoardShim::get_board_data_count (int *result)
{
    return ::get_board_data_count (result);
}

int BoardShim::get_board_data (int data_count, double **data_buf)
{
    float *buf = new float[data_count * num_data_channels];
    double *ts_buf = new double[data_count];

    int res = ::get_board_data (data_count, buf, ts_buf);
    if (res != STATUS_OK)
    {
        delete[] buf;
        delete[] ts_buf;
        return res;
    }
    reshape_data (data_count, buf, ts_buf, data_buf);
    delete[] buf;
    delete[] ts_buf;
    return STATUS_OK;
}

int BoardShim::get_current_board_data (int num_samples, double **data_buf, int *returned_samples)
{
    float *buf = new float[num_samples * num_data_channels];
    double *ts_buf = new double[num_samples];

    int res = ::get_current_board_data (num_samples, buf, ts_buf, returned_samples);
    if (res != STATUS_OK)
    {
        delete[] buf;
        delete[] ts_buf;
        return res;
    }
    reshape_data (*returned_samples, buf, ts_buf, data_buf);
    delete[] buf;
    delete[] ts_buf;
    return STATUS_OK;
}

void BoardShim::reshape_data (int data_count, float *data_buf, double *ts_buf, double **output_buf)
{
    for (int i = 0; i < data_count; i++)
    {
        for (int j = 0; j < num_data_channels; j++)
        {
            output_buf[i][j] = data_buf[i * num_data_channels + j];
        }
        output_buf[i][num_data_channels] = ts_buf[i];
    }
}
