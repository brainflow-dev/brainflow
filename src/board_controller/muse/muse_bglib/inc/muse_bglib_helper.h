#pragma once

#include <ctype.h>
#include <map>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>

#include "cmd_def.h"

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

private:
    static MuseBGLibHelper *instance;
    MuseBGLibHelper ()
    {
    }
    MuseBGLibHelper (const MuseBGLibHelper &other) = delete;
    MuseBGLibHelper &operator= (const MuseBGLibHelper &other) = delete;

    volatile bool should_stop_stream;
    std::thread read_characteristic_thread;

    void thread_worker ();

protected:
    volatile bd_addr connect_addr;
    volatile uint8 connection;
    volatile uint16 muse_handle_start;
    volatile uint16 muse_handle_end;
    volatile int state;
    volatile bool initialized;
    int board_id;
    struct BrainFlowInputParams input_params;
    std::set<uint16> ccids;
    std::map<std::string, uint16> characteristics;

public:
    volatile int exit_code;

    // helpers
    static MuseBGLibHelper *get_instance ();
    void reset ();
    int read_message (int timeout);
    int open_ble_dev ();
    int wait_for_callback ();
    int reset_ble_dev ();

    // methods from dll which will be called
    int initialize (void *param);
    int open_device ();
    int stop_stream ();
    int start_stream ();
    int close_device ();
    int get_data (void *param);
    int release ();
    int config_device (void *param);

    ~MuseBGLibHelper ()
    {
    }

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
};
