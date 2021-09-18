#include <string.h>
#include <string>

#include "ble_lib_board.h"

#include "bluetooth_types.h"
#include "get_dll_dir.h"


DLLLoader *dll_loader = NULL;

BLELibBoard::BLELibBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
}

BLELibBoard::~BLELibBoard ()
{
}

DLLLoader *BLELibBoard::get_dll_loader ()
{
    if (dll_loader != NULL)
    {
        char blelib_dir[1024];
        bool res = get_dll_path (blelib_dir);
        std::string blelib_path = "";
#ifdef _WIN32
        std::string lib_name;
        if (sizeof (void *) == 4)
        {
            lib_name = "simpleble-c.dll";
        }
        else
        {
            lib_name = "simpleble-c32.dll";
        }
#elif defined(__APPLE__)
        std::string lib_name = "libsimpleble-c.dylib";
#else
        std::string lib_name = "libsimpleble-c.so";
#endif
        if (res)
        {
            blelib_path = std::string (blelib_dir) + lib_name;
        }
        else
        {
            blelib_path = lib_name;
        }

        safe_logger (spdlog::level::debug, "use dyn lib: {}", blelib_path.c_str ());
        dll_loader = new DLLLoader (blelib_path.c_str ());
        if (!dll_loader->load_library ())
        {
            safe_logger (spdlog::level::err, "failed to load lib");
            delete dll_loader;
            dll_loader = NULL;
        }
    }

    return dll_loader;
}

int BLELibBoard::bluetooth_open_device ()
{
    int (*func_open) (int, char *) =
        (int (*) (int, char *))dll_loader->get_address ("bluetooth_open_device");
    if (func_open == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for bluetooth_open_device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    int res = func_open (params.ip_port, const_cast<char *> (params.mac_address.c_str ()));
    if (res != (int)SocketBluetoothReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to open bt connection: {}", res);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
