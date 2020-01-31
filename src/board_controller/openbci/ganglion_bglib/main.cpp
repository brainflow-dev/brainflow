#include <chrono>
#include <ctype.h>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "cmd_def.h"
#include "helpers.h"
#include "uart.h"

#include "ganglion_interface.h"

// read Bluetooth_Smart_Software_v1.3.1_API_Reference.pdf to understand this code

namespace GanglionLib
{
    volatile int exit_code = (int)GanglionLib::SYNC_ERROR;
    char uart_port[1024];
    std::queue<struct GanglionLib::GanglionData> data_queue;
    volatile bd_addr connect_addr;
    volatile uint8 connection = -1;
    volatile uint16 ganglion_handle_start = 0;
    volatile uint16 ganglion_handle_end = 0;
    volatile uint16 ganglion_handle_recv = 0;
    volatile uint16 ganglion_handle_send = 0;
    volatile uint16 client_char_handle = 0;
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
                return (int)CustomExitCodes::PORT_OPEN_ERROR;
            }
            strcpy (uart_port, (const char *)param);
            bglib_output = output;
            exit_code = (int)CustomExitCodes::SYNC_ERROR;
            initialized = true;
        }
        return (int)CustomExitCodes::STATUS_OK;
    }

    int open_ganglion (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)CustomExitCodes::PORT_OPEN_ERROR;
        }
        int res = reset_ble_dev ();
        if (res != (int)CustomExitCodes::STATUS_OK)
        {
            return res;
        }
        exit_code = (int)CustomExitCodes::SYNC_ERROR;
        state = State::OPEN_CALLED;
        ble_cmd_gap_discover (gap_discover_observation);

        res = wait_for_callback (15);
        if (res != (int)CustomExitCodes::STATUS_OK)
        {
            return res;
        }
        ble_cmd_gap_end_procedure ();
        return open_ble_dev ();
    }

    int open_ganglion_mac_addr (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)CustomExitCodes::GANGLION_NOT_FOUND_ERROR;
        }
        int res = reset_ble_dev ();
        if (res != (int)CustomExitCodes::STATUS_OK)
        {
            return res;
        }
        exit_code = (int)CustomExitCodes::SYNC_ERROR;
        state = State::OPEN_CALLED;
        char *mac_addr = (char *)param;
        // convert string mac addr to bd_addr struct
        for (int i = 0; i < strlen (mac_addr); i++)
        {
            mac_addr[i] = tolower (mac_addr[i]);
        }
        short unsigned int addr[6];
        if (sscanf (mac_addr, "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx", &addr[5], &addr[4], &addr[3],
                &addr[2], &addr[1], &addr[0]) == 6)
        {
            for (int i = 0; i < 6; i++)
            {
                connect_addr.addr[i] = addr[i];
            }
        }
        else
        {
            return (int)CustomExitCodes::INVALID_MAC_ADDR_ERROR;
        }
        return open_ble_dev ();
    }

    int stop_stream (void *param)
    {
        if (!should_stop_stream)
        {
            should_stop_stream = true;
            read_characteristic_thread.join ();
        }
        int res = config_board ((char *)"s");
        while (!data_queue.empty ())
        {
            data_queue.pop ();
        }
        return res;
    }

    int start_stream (void *param)
    {
        int res = config_board ((char *)"b");
        if (res != (int)CustomExitCodes::STATUS_OK)
        {
            return res;
        }
        // from silicanlabs forum - write 0x00001 to enable notifications
        uint8 configuration[] = {0x01, 0x00};
        state = State::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)GanglionLib::SYNC_ERROR;
        ble_cmd_attclient_attribute_write (connection, client_char_handle, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        res = wait_for_callback (10);
        should_stop_stream = false;
        read_characteristic_thread = std::thread (read_characteristic_worker);
        return res;
    }

    int close_ganglion (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodes::GANGLION_IS_NOT_OPEN_ERROR;
        }
        state = State::CLOSE_CALLED;

        if (!should_stop_stream)
        {
            stop_stream (NULL);
        }

        connection = -1;
        ganglion_handle_start = 0;
        ganglion_handle_end = 0;
        ganglion_handle_recv = 0;
        ganglion_handle_send = 0;

        uart_close ();

        return (int)CustomExitCodes::STATUS_OK;
    }

    int get_data (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodes::GANGLION_IS_NOT_OPEN_ERROR;
        }
        state = State::GET_DATA_CALLED;
        if (data_queue.empty ())
        {
            return (int)CustomExitCodes::NO_DATA_ERROR;
        }
        struct GanglionData *board_data = (struct GanglionData *)param;
        struct GanglionData data = data_queue.front ();
        board_data->timestamp = data.timestamp;
        for (int i = 0; i < 20; i++)
        {
            board_data->data[i] = data.data[i];
        }
        data_queue.pop ();
        return (int)CustomExitCodes::STATUS_OK;
    }

    int config_board (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodes::GANGLION_IS_NOT_OPEN_ERROR;
        }
        exit_code = (int)CustomExitCodes::SYNC_ERROR;
        char *config = (char *)param;
        int len = strlen (config);
        state = State::CONFIG_CALLED;
        if (!ganglion_handle_send)
        {
            return (int)CustomExitCodes::SEND_CHARACTERISTIC_NOT_FOUND_ERROR;
        }
        ble_cmd_attclient_attribute_write (connection, ganglion_handle_send, len, (uint8 *)config);
        ble_cmd_attclient_execute_write (connection, 1);
        int res = wait_for_callback (15);
        return res;
    }

    int release (void *param)
    {
        if (initialized)
        {
            close_ganglion (NULL);
            state = State::NONE;
            initialized = false;
        }
        return (int)CustomExitCodes::STATUS_OK;
    }

} // GanglionLib
