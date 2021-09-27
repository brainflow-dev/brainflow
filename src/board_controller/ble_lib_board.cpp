#include <string.h>
#include <string>

#include "ble_lib_board.h"

#include "bluetooth_types.h"
#include "get_dll_dir.h"


DLLLoader *BLELibBoard::dll_loader = NULL;
std::mutex BLELibBoard::mutex;


BLELibBoard::BLELibBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
}

BLELibBoard::~BLELibBoard ()
{
}

bool BLELibBoard::init_dll_loader ()
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        char blelib_dir[1024];
        bool res = get_dll_path (blelib_dir);
        std::string blelib_path = "";
#ifdef _WIN32
        std::string lib_name;
        if (sizeof (void *) == 4)
        {
            lib_name = "simpleble-c32.dll";
        }
        else
        {
            lib_name = "simpleble-c.dll";
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

        Board::board_logger->debug ("use dyn lib: {}", blelib_path.c_str ());
        BLELibBoard::dll_loader = new DLLLoader (blelib_path.c_str ());
        if (!BLELibBoard::dll_loader->load_library ())
        {
            Board::board_logger->error ("failed to load lib");
            delete BLELibBoard::dll_loader;
            BLELibBoard::dll_loader = NULL;
        }
    }

    return BLELibBoard::dll_loader != NULL;
}

void BLELibBoard::simpleble_free (void *handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return;
    }
    void (*func) (void *) =
        (void (*) (void *))BLELibBoard::dll_loader->get_address ("simpleble_free");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for simpleble_free");
        return;
    }

    return func (handle);
}

size_t BLELibBoard::simpleble_adapter_get_count (void)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return 0;
    }
    size_t (*func) (void) =
        (size_t (*) (void))BLELibBoard::dll_loader->get_address ("simpleble_adapter_get_count");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_adapter_get_count");
        return 0;
    }

    return func ();
}

simpleble_adapter_t BLELibBoard::simpleble_adapter_get_handle (size_t index)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return NULL;
    }
    simpleble_adapter_t (*func) (size_t) = (simpleble_adapter_t (*) (
        size_t))BLELibBoard::dll_loader->get_address ("simpleble_adapter_get_handle");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_adapter_get_handle");
        return NULL;
    }

    return func (index);
}

void BLELibBoard::simpleble_adapter_release_handle (simpleble_adapter_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return;
    }
    void (*func) (simpleble_adapter_t) =
        (void (*) (simpleble_adapter_t))BLELibBoard::dll_loader->get_address (
            "simpleble_adapter_release_handle");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_adapter_release_handle");
        return;
    }

    func (handle);
}

simpleble_err_t BLELibBoard::simpleble_adapter_scan_for (simpleble_adapter_t handle, int timeout_ms)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_adapter_t, int) = (simpleble_err_t (*) (simpleble_adapter_t,
        int))BLELibBoard::dll_loader->get_address ("simpleble_adapter_scan_for");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_adapter_scan_for");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, timeout_ms);
}

simpleble_err_t BLELibBoard::simpleble_adapter_scan_start (simpleble_adapter_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_adapter_t) = (simpleble_err_t (*) (
        simpleble_adapter_t))BLELibBoard::dll_loader->get_address ("simpleble_adapter_scan_start");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_adapter_scan_start");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle);
}

simpleble_err_t BLELibBoard::simpleble_adapter_scan_stop (simpleble_adapter_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_adapter_t) = (simpleble_err_t (*) (
        simpleble_adapter_t))BLELibBoard::dll_loader->get_address ("simpleble_adapter_scan_stop");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_adapter_scan_stop");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle);
}

simpleble_err_t BLELibBoard::simpleble_adapter_set_callback_on_scan_updated (
    simpleble_adapter_t handle,
    void (*callback) (
        simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *userdata),
    void *userdata)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_adapter_t,
        void (*) (simpleble_adapter_t, simpleble_peripheral_t, void *), void *) =
        (simpleble_err_t (*) (simpleble_adapter_t,
            void (*) (simpleble_adapter_t, simpleble_peripheral_t, void *), void *))
            BLELibBoard::dll_loader->get_address ("simpleble_adapter_set_callback_on_scan_updated");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_adapter_set_callback_on_scan_updated");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, callback, userdata);
}

simpleble_err_t BLELibBoard::simpleble_adapter_set_callback_on_scan_found (
    simpleble_adapter_t handle,
    void (*callback) (
        simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *userdata),
    void *userdata)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_adapter_t,
        void (*) (simpleble_adapter_t, simpleble_peripheral_t, void *), void *) =
        (simpleble_err_t (*) (simpleble_adapter_t,
            void (*) (simpleble_adapter_t, simpleble_peripheral_t, void *), void *))
            BLELibBoard::dll_loader->get_address ("simpleble_adapter_set_callback_on_scan_found");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_adapter_set_callback_on_scan_found");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, callback, userdata);
}

char *BLELibBoard::simpleble_peripheral_address (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return NULL;
    }
    char *(*func) (simpleble_peripheral_t) =
        (char *(*)(simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_address");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_peripheral_address");
        return NULL;
    }

    return func (handle);
}

char *BLELibBoard::simpleble_peripheral_identifier (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return NULL;
    }
    char *(*func) (simpleble_peripheral_t) =
        (char *(*)(simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_identifier");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_identifier");
        return NULL;
    }

    return func (handle);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_connect (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t) =
        (simpleble_err_t (*) (simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_connect");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_peripheral_connect");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_disconnect (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t) =
        (simpleble_err_t (*) (simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_disconnect");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_disconnect");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle);
}

void BLELibBoard::simpleble_peripheral_release_handle (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return;
    }
    void (*func) (simpleble_peripheral_t) =
        (void (*) (simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_release_handle");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_release_handle");
        return;
    }

    func (handle);
}

size_t BLELibBoard::simpleble_peripheral_services_count (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return 0;
    }
    size_t (*func) (simpleble_peripheral_t) =
        (size_t (*) (simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_services_count");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_services_count");
        return 0;
    }

    return func (handle);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_services_get (
    simpleble_peripheral_t handle, size_t index, simpleble_service_t *services)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t, size_t, simpleble_service_t *) =
        (simpleble_err_t (*) (simpleble_peripheral_t, size_t, simpleble_service_t *))
            BLELibBoard::dll_loader->get_address ("simpleble_peripheral_services_get");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_services_get");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, index, services);
}


simpleble_err_t BLELibBoard::simpleble_peripheral_write_request (simpleble_peripheral_t handle,
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t data_length)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (
        simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t, uint8_t *, size_t) =
        (simpleble_err_t (*) (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t, uint8_t *,
            size_t))BLELibBoard::dll_loader->get_address ("simpleble_peripheral_write_request");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_write_request");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, service, characteristic, data, data_length);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_write_command (simpleble_peripheral_t handle,
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t data_length)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (
        simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t, uint8_t *, size_t) =
        (simpleble_err_t (*) (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t, uint8_t *,
            size_t))BLELibBoard::dll_loader->get_address ("simpleble_peripheral_write_command");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_write_command");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, service, characteristic, data, data_length);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_notify (simpleble_peripheral_t handle,
    simpleble_uuid_t service, simpleble_uuid_t characteristic,
    void (*callback) (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t data_length, void *userdata),
    void *userdata)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t,
        void (*) (simpleble_uuid_t, simpleble_uuid_t, uint8_t *, size_t, void *), void *) =
        (simpleble_err_t (*) (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t,
            void (*) (simpleble_uuid_t, simpleble_uuid_t, uint8_t *, size_t, void *),
            void *))BLELibBoard::dll_loader->get_address ("simpleble_peripheral_notify");
    if (func == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for simpleble_peripheral_notify");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, service, characteristic, callback, userdata);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_unsubscribe (
    simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t, simpleble_uuid_t,
        simpleble_uuid_t) = (simpleble_err_t (*) (simpleble_peripheral_t, simpleble_uuid_t,
        simpleble_uuid_t))BLELibBoard::dll_loader->get_address ("simpleble_peripheral_unsubscribe");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_unsubscribe");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, service, characteristic);
}

size_t BLELibBoard::simpleble_peripheral_manufacturer_data_count (simpleble_peripheral_t handle)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return 0;
    }
    size_t (*func) (simpleble_peripheral_t) =
        (size_t (*) (simpleble_peripheral_t))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_manufacturer_data_count");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_manufacturer_data_count");
        return 0;
    }

    return func (handle);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_manufacturer_data_get (
    simpleble_peripheral_t handle, size_t index, simpleble_manufacturer_data_t *manufacturer_data)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t, size_t, simpleble_manufacturer_data_t *) =
        (simpleble_err_t (*) (simpleble_peripheral_t, size_t, simpleble_manufacturer_data_t *))
            BLELibBoard::dll_loader->get_address ("simpleble_peripheral_manufacturer_data_get");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_manufacturer_data_get");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, index, manufacturer_data);
}

simpleble_err_t BLELibBoard::simpleble_peripheral_is_connected (
    simpleble_peripheral_t handle, bool *connected)
{
    std::lock_guard<std::mutex> lock (BLELibBoard::mutex);
    if (BLELibBoard::dll_loader == NULL)
    {
        safe_logger (spdlog::level::err, "BLELibBoard::dll_loader is not initialized");
        return SIMPLEBLE_FAILURE;
    }
    simpleble_err_t (*func) (simpleble_peripheral_t, bool *) =
        (simpleble_err_t (*) (simpleble_peripheral_t, bool *))BLELibBoard::dll_loader->get_address (
            "simpleble_peripheral_is_connected");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for simpleble_peripheral_is_connected");
        return SIMPLEBLE_FAILURE;
    }

    return func (handle, connected);
}
