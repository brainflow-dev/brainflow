#include <string>
#include <utility>

#include "get_dll_dir.h"
#include "muse_s_bled.h"
#include "muse_types.h"

#include "brainflow_constants.h"


int MuseSBLED::num_objects = 0;


MuseSBLED::MuseSBLED (struct BrainFlowInputParams params)
    : DynLibBoard<10> ((int)BoardIds::MUSE_S_BLED_BOARD, params)
{
    MuseSBLED::num_objects++;

    if (MuseSBLED::num_objects > 1)
    {
        is_valid = false;
    }
    else
    {
        is_valid = true;
    }
}

MuseSBLED::~MuseSBLED ()
{
    skip_logs = true;
    MuseSBLED::num_objects--;
    release_session ();
}

std::string MuseSBLED::get_lib_name ()
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

int MuseSBLED::prepare_session ()
{
    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one MuseSBLED per process is allowed");
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "you need to specify dongle port");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return DynLibBoard<10>::prepare_session ();
}

int MuseSBLED::call_init ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("initialize");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for initialize");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    std::pair<int, struct BrainFlowInputParams> info = std::make_pair (board_id, params);
    int res = func ((void *)&info);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to initialize {}", res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
