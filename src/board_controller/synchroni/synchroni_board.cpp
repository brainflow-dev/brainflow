#include "synchroni_board.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"
#include <string.h>



static void synchroni_adapter_1_on_scan_found(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((SynchroniBoard *)(board))->adapter_1_on_scan_found (adapter, peripheral);
}


SynchroniBoard::SynchroniBoard (struct BrainFlowInputParams params)
: BLELibBoard ((int)BoardIds::SYNCHRONI_BOARD,params)
{
    initialized = false;
    is_streaming = false;
}

SynchroniBoard::~SynchroniBoard()
{
    skip_logs = true;
    release_session();
}

int SynchroniBoard::prepare_session()
{
    return 0;
}

int SynchroniBoard::start_stream(int buffer_size, const char * streamer_params)
{
        if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("0a8100000d");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::debug, "Start command Send");
        is_streaming = true;
    }

    return res;
}


int SynchroniBoard::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int SynchroniBoard::config_board (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }


    safe_logger (spdlog::level::trace, config[2]);
    
    if (simpleble_peripheral_write_command (synchroni_peripheral, write_characteristics.first,
            write_characteristics.second, config, sizeof (config)) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}


void SynchroniBoard::read_data(simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t size, int channel_num)
{
    if (size != synchroni_packet_size)
    {
        safe_logger (spdlog::level::warn, "unexpected number of bytes received: {}", size);
        return;
    }
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];

    for (int i = 0; i < num_rows; i++){
        int num_rows = board_descr["default"]["num_rows"];
            double *package = new double[num_rows];
        for (int i = 0; i < num_rows; i+=2)
        {
            package[i] = 0.0;
        }
        std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
        std::vector<int> accel_channels = board_descr["default"]["accel_channels"];
        std::vector<int> gyro_channels = board_descr["default"]["gyro_channels"];
        package[board_descr["default"]["package_num_channel"].get<int> ()] =
                data[synchroni_packet_size + i];

        for(int j = i + synchroni_axl_start_index, k =0; j <i +synchroni_axl_end_index;j += 2, k++)
        {
            package[accel_channels[k]] = (data[j] << 8) | data[j + 1];
            if (package[accel_channels[k]] > 32767)
                package[accel_channels[k]] = package[accel_channels[k]] - 65535;
        }
        
        for (int j = i + synchroni_gyro_start_index, k = 0; j < i + synchroni_gyro_end_index;
                 j += 2, k++)
            {
                package[gyro_channels[k]] = (data[j] << 8) | data[j + 1];
                if (package[gyro_channels[k]] > 32767)
                    package[gyro_channels[k]] = package[gyro_channels[k]] - 65535;
            }

        package[board_descr["default"]["marker_channel"].get<int> ()] =
            data[(brainalive_packet_index + 1) + i];
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (&package[0]);
        
    }
    return;
}
void SynchroniBoard::adapter_1_on_scan_found(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
{
    char *peripheral_indentified = simpleble_peripheral_identifier(peripheral);
    char *peripheral_address = simpleble_peripheral_address (peripheral);
    bool found = false;
    if(!params.mac_address.empty())
    {
        if (strcmp(peripheral_address, params.mac_address.c_str()) == 0){
            found = true;
        }
    }
    else
    {
        if (strcmp(peripheral_indentified, "",7) == 0){
            found = true;
        }
    }
    safe_logger (spdlog::level::trace, "address {}", peripheral_address);
    simpleble_free (peripheral_address);
    safe_logger (spdlog::level::trace, "identifier {}", peripheral_identified);
    simpleble_free (peripheral_identified);
        if (found)
    {
        {
            std::lock_guard<std::mutex> lk (m);
            synchroni_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}