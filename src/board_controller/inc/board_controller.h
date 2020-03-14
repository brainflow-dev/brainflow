#pragma once

#include <string>
#include <tuple>

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT __attribute__((visibility("default")))
#define CALLING_CONVENTION
#endif

#include "board_info_getter.h"
#include "brainflow_constants.h"


enum class IpProtocolType
{
    NONE = 0,
    UDP = 1,
    TCP = 2
};

// we pass this structure from user API as a json string
struct BrainFlowInputParams
{
    std::string serial_port;
    std::string mac_address;
    std::string ip_address;
    int ip_port;
    int ip_protocol;
    std::string other_info;

    BrainFlowInputParams ()
    {
        serial_port = "";
        mac_address = "";
        ip_address = "";
        ip_port = 0;
        ip_protocol = (int)IpProtocolType::NONE;
        other_info = "";
    }

    // default copy constructor and assignment operator are ok, need less operator to use in map
    bool operator< (const struct BrainFlowInputParams &other) const
    {
        return std::tie (serial_port, mac_address, ip_address, ip_port, ip_protocol, other_info) <
            std::tie (other.serial_port, other.mac_address, other.ip_address, other.ip_port,
                other.ip_protocol, other.other_info);
    }
};

#ifdef __cplusplus
extern "C"
{
#endif
    // I dont use const char * because I am not sure that all
    // languages support passing const char * instead char *

    // data acquisition methods
    SHARED_EXPORT int CALLING_CONVENTION prepare_session (
        int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION start_stream (
        int buffer_size, char *streamer_params, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION stop_stream (
        int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION release_session (
        int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int get_current_board_data (int num_samples, double *data_buf,
        int *returned_samples, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION get_board_data_count (
        int *result, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION get_board_data (
        int data_count, double *data_buf, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION config_board (
        char *config, int board_id, char *json_brainflow_input_params);
    SHARED_EXPORT int CALLING_CONVENTION is_prepared (
        int *prepared, int board_id, char *json_brainflow_input_params);

    // logging methods
    SHARED_EXPORT int CALLING_CONVENTION set_log_level (int log_level);
    SHARED_EXPORT int CALLING_CONVENTION set_log_file (char *log_file);
    SHARED_EXPORT int CALLING_CONVENTION log_message (int log_level, char *message);

#ifdef __cplusplus
}
#endif