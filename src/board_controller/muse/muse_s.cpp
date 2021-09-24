#include "muse_s.h"


void adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((MuseS *)(board))->adapter_on_scan_found (adapter, peripheral);
}

MuseS::MuseS (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::MUSE_S_BOARD, params)
{
    keep_alive = false;
    initialized = false;
    muse_adapter = NULL;
}

MuseS::~MuseS ()
{
    skip_logs = true;
    release_session ();
}

int MuseS::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.timeout < 1)
    {
        params.timeout = 5;
    }
    safe_logger (spdlog::level::info, "Use timeout for discovery: {}", params.timeout);
    if (!init_dll_loader ())
    {
        safe_logger (spdlog::level::err, "Failed to init dll_loader");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    size_t adapter_count = simpleble_adapter_get_count ();
    if (adapter_count == 0)
    {
        safe_logger (spdlog::level::err, "No BLE adapters found");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    muse_adapter = simpleble_adapter_get_handle (0);
    if (muse_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_found (
        muse_adapter, ::adapter_on_scan_found, (void *)this);

    simpleble_adapter_scan_for (muse_adapter, params.timeout * 1000);

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::start_stream (int buffer_size, const char *streamer_params)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::stop_stream ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::release_session ()
{
    if (muse_adapter != NULL)
    {
        simpleble_adapter_release_handle (muse_adapter);
        muse_adapter = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void MuseS::read_thread ()
{
}

void MuseS::adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
{
    char *peripheral_identifier = simpleble_peripheral_identifier (peripheral);
    char *peripheral_address = simpleble_peripheral_address (peripheral);

    safe_logger (spdlog::level::info, "identifier {}", peripheral_identifier);

    safe_logger (spdlog::level::info, "address {}", peripheral_address);

    free (peripheral_identifier);
    free (peripheral_address);
    simpleble_peripheral_release_handle (peripheral);
}
