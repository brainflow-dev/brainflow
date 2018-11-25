#ifdef _WIN32
#include <windows.h>
#endif

#include "board_controller.h"
#include "cython.h"
#include "board.h"


Board *board = NULL;

int prepare_session (int board_id, char *port_name)
{
    if (board)
        return STATUS_OK;

    int res = STATUS_OK;
    switch (board_id)
    {
        case CYTHON_BOARD:
            board = new Cython (port_name);
            res = board->prepare_session ();
            break;
        default:
            return UNSUPPORTED_BOARD_ERROR;
    }
    if (res != STATUS_OK)
    {
        delete board;
        board = NULL;
    }
    return res;
}

int start_stream (int buffer_size)
{
    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->start_stream (buffer_size);
}

int stop_stream ()
{
    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->stop_stream ();
}

int release_session ()
{
    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    int res = board->release_session ();
    delete board;
    board = NULL;

    return res;
}

int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples)
{
    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->get_current_board_data (num_samples, data_buf, ts_buf, returned_samples);
}

int get_board_data_count (int *result)
{
    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->get_board_data_count (result);
}

int get_board_data (int data_count, float *data_buf, double *ts_buf)
{
    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->get_board_data (data_count, data_buf, ts_buf);
}

#ifdef _WIN32
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_DETACH:
        {
            if (board != NULL)
                release_session ();
            break;
        }
        default:
            break;
    }
    return TRUE;
}
#else
__attribute__((destructor)) static void terminate_all (void)
{
    if (board != NULL)
        release_session ();
}
#endif
