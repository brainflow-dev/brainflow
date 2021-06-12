#pragma once

#include <ctype.h>
#include <map>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>

#include "cmd_def.h"

#include "brainflow_constants.h"
#include "brainflow_input_params.h"


enum class DeviceState : int
{
    NONE = 0,
    INIT_CALLED = 1,
    INITIAL_CONNECTION = 2,
    OPEN_CALLED = 3,
    CONFIG_CALLED = 4,
    CLOSE_CALLED = 5,
    GET_DATA_CALLED = 6,
    WRITE_TO_CLIENT_CHAR = 7
};

class MuseBGLibHelper
{

protected:
    volatile bool should_stop_stream;
    std::thread read_characteristic_thread;

    volatile bd_addr connect_addr;
    volatile uint8 connection;
    volatile uint16 muse_handle_start;
    volatile uint16 muse_handle_end;
    volatile int state;
    volatile bool initialized;
    struct BrainFlowInputParams input_params;
    std::set<uint16> ccids;
    std::map<std::string, uint16> characteristics;

    void thread_worker ();

public:
    volatile int exit_code;

    MuseBGLibHelper ()
    {
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        connection = -1;
        muse_handle_start = 0;
        muse_handle_end = 0;
        state = (int)DeviceState::NONE;
        should_stop_stream = true;
        initialized = false;
    }

    virtual ~MuseBGLibHelper ()
    {
    }

    MuseBGLibHelper (const MuseBGLibHelper &other) = delete;
    MuseBGLibHelper &operator= (const MuseBGLibHelper &other) = delete;

    // methods from dll which will be called
    virtual int initialize (struct BrainFlowInputParams params);
    virtual int open_device ();
    virtual int stop_stream ();
    virtual int start_stream ();
    virtual int close_device ();
    virtual int get_data (void *param);
    virtual int release ();
    virtual int config_device (const char *config);

    // callbacks from bglib which we need
    virtual void ble_evt_connection_status (const struct ble_msg_connection_status_evt_t *msg);
    virtual void ble_evt_connection_disconnected (
        const struct ble_msg_connection_disconnected_evt_t *msg);
    virtual void ble_evt_attclient_group_found (
        const struct ble_msg_attclient_group_found_evt_t *msg);
    virtual void ble_evt_attclient_procedure_completed (
        const struct ble_msg_attclient_procedure_completed_evt_t *msg);
    virtual void ble_evt_attclient_find_information_found (
        const struct ble_msg_attclient_find_information_found_evt_t *msg);
    virtual void ble_evt_attclient_attribute_value (
        const struct ble_msg_attclient_attribute_value_evt_t *msg);
    virtual void ble_evt_gap_scan_response (const struct ble_msg_gap_scan_response_evt_t *msg);

    // helpers
    void reset ();
    int read_message (int timeout);
    int open_ble_dev ();
    int wait_for_callback ();
    int reset_ble_dev ();
};
