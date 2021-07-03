#include <array>
#include <chrono>
#include <ctype.h>
#include <deque>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "cmd_def.h"
#include "helpers.h"
#include "uart.h"

#include "ticket_lock.h"

#include "brainbit_functions.h"

#include "brainflow_constants.h"
#include "brainflow_input_params.h"

#include "json.hpp"
using json = nlohmann::json;

// read Bluetooth_Smart_Software_v1.3.1_API_Reference.pdf to understand this code

namespace BrainBitBLEDLib
{
    volatile int exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    char uart_port[1024];
    int timeout = 15;

    std::deque<std::array<double, BRAINBIT_BLED_DATA_SIZE>> data_queue;
    TicketLock lock;
    volatile bd_addr connect_addr;

    volatile uint8 connection = -1;
    volatile uint16 brainbit_handle_start = 0;
    volatile uint16 brainbit_handle_end = 0;
    volatile uint16 brainbit_handle_recv = 0;
    volatile uint16 brainbit_handle_send = 0;
    volatile uint16 brainbit_handle_status = 0;
    volatile double battery_level = 0.0;
    std::set<uint16> ccids;

    volatile State state =
        State::NONE; // same callbacks are triggered by different methods we need to differ them
    volatile bool should_stop_stream = true;

    bool initialized = false;
    std::thread read_characteristic_thread;

    void read_characteristic_worker ()
    {
        while (!should_stop_stream)
        {
            read_message (UART_TIMEOUT);
        }
    }

    int initialize (void *param)
    {
        if (!initialized)
        {
            if (param == NULL)
            {
                return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
            }
            std::tuple<int, struct BrainFlowInputParams, json> *info =
                (std::tuple<int, struct BrainFlowInputParams, json> *)param;
            BrainFlowInputParams params = std::get<1> (*info);
            strcpy (uart_port, params.serial_port.c_str ());
            timeout = params.timeout;
            bglib_output = output;
            exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
            initialized = true;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    int open_device (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
        }
        int res = reset_ble_dev ();
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        state = State::OPEN_CALLED;
        ble_cmd_gap_discover (gap_discover_observation);
        res = wait_for_callback (timeout);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        ble_cmd_gap_end_procedure ();
        return open_ble_dev ();
    }

    int open_device_mac_addr (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
        int res = reset_ble_dev ();
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        state = State::OPEN_CALLED;
        char *mac_addr = (char *)param;
        // convert string mac addr to bd_addr struct
        for (unsigned int i = 0; i < strlen (mac_addr); i++)
        {
            mac_addr[i] = tolower (mac_addr[i]);
        }
        short unsigned int addr[6];
        if (sscanf (mac_addr, "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx", &addr[5], &addr[4], &addr[3],
                &addr[2], &addr[1], &addr[0]) == 6)
        {
            for (int i = 0; i < 6; i++)
            {
                connect_addr.addr[i] = (uint8)addr[i];
            }
        }
        else
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        return open_ble_dev ();
    }

#ifdef __linux__
    int stop_stream (void *param)
    {
        // dirty hack to solve https://github.com/brainflow-dev/brainflow/issues/24
        if (!initialized)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        if (!should_stop_stream)
        {
            should_stop_stream = true;
            read_characteristic_thread.join ();
        }
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        state = State::CONFIG_CALLED;
        if (!brainbit_handle_send)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        volatile bool stop_config_thread = false;
        std::thread config_thread = std::thread ([&] () {
            uint8 signal_command[] = {0x01};
            while (!stop_config_thread)
            {
                ble_cmd_attclient_attribute_write (
                    connection, brainbit_handle_send, 1, signal_command);
                ble_cmd_attclient_execute_write (connection, 1);
            }
        });
        int res = wait_for_callback (timeout);
        stop_config_thread = true;
        config_thread.join ();
        // need to remove generated by thread above messages from serial port buffer
        int l = 1;
        unsigned char temp_data;
        while (l > 0)
        {
            l = uart_rx (l, &temp_data, 1000);
        }
        data_queue.clear ();
        return res;
    }
#else
    int stop_stream (void *param)
    {
        // hack above doesnt work well on windows and not needed for windows/macos
        if (!should_stop_stream)
        {
            should_stop_stream = true;
            read_characteristic_thread.join ();
        }
        uint8 signal_command[] = {0x01}; // from brainbit web
        int res = config_board (signal_command, 1);
        data_queue.clear ();
        return res;
    }
#endif

    int start_stream (void *param)
    {
        // from silicanlabs forum - write 0x00001 to enable notifications
        uint8 configuration[] = {0x01, 0x00};
        for (uint16 ccid : ccids)
        {
            state = State::WRITE_TO_CLIENT_CHAR;
            exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
            ble_cmd_attclient_attribute_write (connection, ccid, 2, &configuration);
            ble_cmd_attclient_execute_write (connection, 1);
            int res = wait_for_callback (timeout);
            if (res != (int)BrainFlowExitCodes::STATUS_OK)
            {
                return res;
            }
        }

        // from brainbit web
        uint8 signal_command[] = {0x02, 0x00, 0x00, 0x00, 0x00};
        state = State::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        ble_cmd_attclient_attribute_write (connection, brainbit_handle_send, 5, &signal_command);
        ble_cmd_attclient_execute_write (connection, 1);
        int res = wait_for_callback (timeout);

        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            should_stop_stream = false;
            read_characteristic_thread = std::thread (read_characteristic_worker);
        }

        return res;
    }

    int close_device (void *param)
    {
        if (!initialized)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        state = State::CLOSE_CALLED;

        if (!should_stop_stream)
        {
            stop_stream (NULL);
        }

        connection = -1;
        brainbit_handle_start = 0;
        brainbit_handle_end = 0;
        brainbit_handle_recv = 0;
        brainbit_handle_send = 0;

        uart_close ();

        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    int get_data (void *param)
    {
        if (!initialized)
        {
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        if (should_stop_stream)
        {
            return (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
        }
        state = State::GET_DATA_CALLED;
        int res = (int)BrainFlowExitCodes::STATUS_OK;
        lock.lock ();
        if (data_queue.empty ())
        {
            res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
        }
        else
        {
            try
            {
                double *board_data = (double *)param;
                std::array<double, BRAINBIT_BLED_DATA_SIZE> data = data_queue.at (
                    0); // at ensures out of range exception, front has undefined behavior
                for (int i = 0; i < BRAINBIT_BLED_DATA_SIZE; i++)
                {
                    board_data[i] = data[i];
                }
                data_queue.pop_front ();
            }
            catch (...)
            {
                res = (int)BrainFlowExitCodes::EMPTY_BUFFER_ERROR;
            }
        }
        lock.unlock ();
        return res;
    }

    int release (void *param)
    {
        if (initialized)
        {
            close_device (NULL);
            state = State::NONE;
            initialized = false;
            data_queue.clear ();
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    int config_device (void *param)
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
    }

} // BrainBitBLEDLib
