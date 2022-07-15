#include <string>
#include <utility>

#include "get_dll_dir.h"
#include "muse_bled.h"

#include "brainflow_constants.h"


int MuseBLED::num_objects = 0;


MuseBLED::MuseBLED (int board_id, struct BrainFlowInputParams params)
    : DynLibBoard (board_id, params)
{
    MuseBLED::num_objects++;

    if (MuseBLED::num_objects > 1)
    {
        is_valid = false;
    }
    else
    {
        is_valid = true;
    }
}

MuseBLED::~MuseBLED ()
{
    skip_logs = true;
    MuseBLED::num_objects--;
    release_session ();
}

std::string MuseBLED::get_lib_name ()
{
    std::string muselib_path = "";
    std::string muselib_name = "";
    char muselib_dir[1024];
    bool res = get_dll_path (muselib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        muselib_name = "MuseLib32.dll";
    }
    else
    {
        muselib_name = "MuseLib.dll";
    }
#endif
#ifdef __linux__
    muselib_name = "libMuseLib.so";
#endif
#ifdef __APPLE__
    muselib_name = "libMuseLib.dylib";
#endif

    if (res)
    {
        muselib_path = std::string (muselib_dir) + muselib_name;
    }
    else
    {
        muselib_path = muselib_name;
    }
    return muselib_path;
}

int MuseBLED::prepare_session ()
{
    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one MuseBLED per process is allowed");
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "you need to specify dongle port");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return DynLibBoard::prepare_session ();
}

void MuseBLED::read_thread ()
{
    int num_attempts = 0;
    int sleep_time = 10;
    int max_attempts = params.timeout * 1000 / sleep_time;

    int (*func_default) (void *) = (int (*) (void *))dll_loader->get_address ("get_data_default");
    if (func_default == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data_default");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }
    int (*func_aux) (void *) = (int (*) (void *))dll_loader->get_address ("get_data_aux");
    if (func_aux == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data_aux");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }
    int (*func_anc) (void *) = (int (*) (void *))dll_loader->get_address ("get_data_anc");
    if (func_anc == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for get_data_anc");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }

    int num_rows_default = board_descr["default"]["num_rows"];
    double *data_default = new double[num_rows_default];
    if (data_default == NULL)
    {
        safe_logger (spdlog::level::err, "failed to allocate data");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }
    for (int i = 0; i < num_rows_default; i++)
    {
        data_default[i] = 0.0;
    }

    int num_rows_aux = board_descr["auxiliary"]["num_rows"];
    double *data_aux = new double[num_rows_aux];
    if (data_aux == NULL)
    {
        safe_logger (spdlog::level::err, "failed to allocate data");
        state = (int)BrainFlowExitCodes::GENERAL_ERROR;
        return;
    }
    for (int i = 0; i < num_rows_aux; i++)
    {
        data_aux[i] = 0.0;
    }

    double *data_anc = NULL;
    if (board_id != (int)BoardIds::MUSE_2016_BLED_BOARD)
    {
        int num_rows_anc = board_descr["ancillary"]["num_rows"];
        data_anc = new double[num_rows_anc];
        if (data_anc == NULL)
        {
            safe_logger (spdlog::level::err, "failed to allocate data");
            state = (int)BrainFlowExitCodes::GENERAL_ERROR;
            return;
        }
        for (int i = 0; i < num_rows_anc; i++)
        {
            data_anc[i] = 0.0;
        }
    }

    while (keep_alive)
    {
        int res = func_aux ((void *)data_aux);
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            push_package (data_aux, (int)BrainFlowPresets::AUXILIARY_PRESET);
        }
        if (data_anc != NULL)
        {
            res = func_anc ((void *)data_anc);
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                push_package (data_anc, (int)BrainFlowPresets::ANCILLARY_PRESET);
            }
        }
        res = func_default ((void *)data_default);
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            if (state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                cv.notify_one ();
            }
            push_package (data_default);
        }
        else
        {
            if (state == (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR)
            {
                num_attempts++;
            }
            if (num_attempts == max_attempts)
            {
                safe_logger (spdlog::level::err, "no data received");
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::GENERAL_ERROR;
                }
                cv.notify_one ();
                break;
            }
#ifdef _WIN32
            Sleep (sleep_time);
#else
            usleep (sleep_time * 1000);
#endif
        }
    }
    delete[] data_default;
    delete[] data_aux;
    if (data_anc != NULL)
    {
        delete[] data_anc;
    }
}