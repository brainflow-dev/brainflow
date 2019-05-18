#ifdef _WIN32
#include <windows.h>
#endif

#include <mutex>

#include "board.h"
#include "board_controller.h"
#include "cyton.h"

Board *board = NULL;
std::mutex mutex;

int prepare_session (int board_id, char *port_name)
{
    std::lock_guard<std::mutex> lock (mutex);

    if (board)
        return STATUS_OK;

    int res = STATUS_OK;
    switch (board_id)
    {
        case CYTON_BOARD:
            board = new Cyton (port_name);
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
    std::lock_guard<std::mutex> lock (mutex);

    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->start_stream (buffer_size);
}

int stop_stream ()
{
    std::lock_guard<std::mutex> lock (mutex);

    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->stop_stream ();
}

int release_session ()
{
    std::lock_guard<std::mutex> lock (mutex);

    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    int res = board->release_session ();
    delete board;
    board = NULL;

    return res;
}

int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples)
{
    std::lock_guard<std::mutex> lock (mutex);

    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->get_current_board_data (num_samples, data_buf, ts_buf, returned_samples);
}

int get_board_data_count (int *result)
{
    std::lock_guard<std::mutex> lock (mutex);

    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->get_board_data_count (result);
}

int get_board_data (int data_count, float *data_buf, double *ts_buf)
{
    std::lock_guard<std::mutex> lock (mutex);

    if (board == NULL)
        return BOARD_NOT_CREATED_ERROR;

    return board->get_board_data (data_count, data_buf, ts_buf);
}

int set_log_level (int log_level)
{
    std::lock_guard<std::mutex> lock (mutex);
    return Board::set_log_level (log_level);
}

// DLLMain is executed during LoadLibrary\FreeLibrary. Board object desctructorshould be called even
// without it but for sanity check to ensure that we stop streaming data from the board I call it
// manually here as well
#ifdef _WIN32
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_DETACH:
        {
            if (board != NULL)
            {
                board->release_session ();
                delete board;
                board = NULL;
            }
            break;
        }
        default:
            break;
    }
    return TRUE;
}
#else
// the same as DLL_PROCESS_DETACH for linux
__attribute__ ((destructor)) static void terminate_all (void)
{
    if (board != NULL)
    {
        board->release_session ();
        delete board;
        board = NULL;
    }
}
#endif
