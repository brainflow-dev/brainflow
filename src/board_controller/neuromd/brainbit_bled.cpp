#include <string>

#include "brainbit_bled.h"
#include "get_dll_dir.h"

#include "brainflow_constants.h"


int BrainBitBLED::num_objects = 0;


BrainBitBLED::BrainBitBLED (struct BrainFlowInputParams params)
    : DynLibBoard ((int)BoardIds::BRAINBIT_BLED_BOARD, params)
{
    BrainBitBLED::num_objects++;

    if (BrainBitBLED::num_objects > 1)
    {
        is_valid = false;
    }
    else
    {
        is_valid = true;
    }
    use_mac_addr = (params.mac_address.empty ()) ? false : true;
}

BrainBitBLED::~BrainBitBLED ()
{
    skip_logs = true;
    BrainBitBLED::num_objects--;
    release_session ();
}

std::string BrainBitBLED::get_lib_name ()
{
    std::string brainbitlib_path = "";
    std::string brainbitlib_name = "";
    char brainbitlib_dir[1024];
    bool res = get_dll_path (brainbitlib_dir);

#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        brainbitlib_name = "BrainBitLib32.dll";
    }
    else
    {
        brainbitlib_name = "BrainBitLib.dll";
    }
#endif
#ifdef __linux__
    brainbitlib_name = "libBrainBitLib.so";
#endif
#ifdef __APPLE__
    brainbitlib_name = "libBrainBitLib.dylib";
#endif

    if (res)
    {
        brainbitlib_path = std::string (brainbitlib_dir) + brainbitlib_name;
    }
    else
    {
        brainbitlib_path = brainbitlib_name;
    }
    return brainbitlib_path;
}

int BrainBitBLED::prepare_session ()
{
    if (!is_valid)
    {
        safe_logger (spdlog::level::info, "only one BrainBitBLED per process is allowed");
        return (int)BrainFlowExitCodes::ANOTHER_BOARD_IS_CREATED_ERROR;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "you need to specify dongle port");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    return DynLibBoard::prepare_session ();
}

int BrainBitBLED::call_open ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    if (use_mac_addr)
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_device_mac_addr");
        if (func == NULL)
        {
            safe_logger (
                spdlog::level::err, "failed to get function address for open_device_mac_addr");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        safe_logger (spdlog::level::info, "search for {}", params.mac_address.c_str ());
        res = func (const_cast<char *> (params.mac_address.c_str ()));
    }
    else
    {
        int (*func) (void *) = (int (*) (void *))dll_loader->get_address ("open_device");
        if (func == NULL)
        {
            safe_logger (spdlog::level::err, "failed to get function address for open_device");
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        safe_logger (
            spdlog::level::info, "mac address is not specified, try to find brainbit without it");
        res = func (NULL);
    }
    return res;
}
