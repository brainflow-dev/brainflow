#include <chrono>
#include <ctype.h>
#include <mutex>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "cmd_def.h"
#include "helpers.h"
#include "uart.h"

#include "GanglionNativeInterface.h"

// read Bluetooth_Smart_Software_v1.3.1_API_Reference.pdf to understand this code

volatile int exit_code = (int)GanglionLibNative::SYNC_ERROR;
char uart_port[1024];
std::queue<struct GanglionLibNative::GanglionDataNative> data_queue;
volatile bd_addr connect_addr;
volatile uint8 connection = -1;
volatile uint16 ganglion_handle_start = 0;
volatile uint16 ganglion_handle_end = 0;
volatile uint16 ganglion_handle_recv = 0;
volatile uint16 ganglion_handle_send = 0;
volatile uint16 client_char_handle = 0;
volatile State state =
    State::none; // same callbacks are triggered by different methods we need to differ them

bool initialized = false;
std::mutex mutex;
std::thread read_characteristic_thread;
bool should_stop_stream = true;

void read_characteristic_worker ()
{
    while (!should_stop_stream)
    {
        read_message (UART_TIMEOUT);
    }
}

namespace GanglionLibNative
{
    int initialize_native (void *param)
    {
        if (!initialized)
        {
            if (param == NULL)
            {
                return (int)CustomExitCodesNative::PORT_OPEN_ERROR;
            }
            strcpy (uart_port, (const char *)param);
            bglib_output = output;
            exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
            initialized = true;
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int open_ganglion_native (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)CustomExitCodesNative::PORT_OPEN_ERROR;
        }
        int res = reset_ble_dev ();
        if (res != (int)CustomExitCodesNative::STATUS_OK)
        {
            return res;
        }
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        state = State::open_called;
        ble_cmd_gap_discover (gap_discover_observation);

        res = wait_for_callback (15);
        if (res != (int)CustomExitCodesNative::STATUS_OK)
        {
            return res;
        }
        ble_cmd_gap_end_procedure ();
        return open_ble_dev ();
    }

    int open_ganglion_mac_addr_native (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)CustomExitCodesNative::GANGLION_NOT_FOUND_ERROR;
        }
        int res = reset_ble_dev ();
        if (res != (int)CustomExitCodesNative::STATUS_OK)
        {
            return res;
        }
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        state = State::open_called;
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
            return (int)CustomExitCodesNative::INVALID_MAC_ADDR_ERROR;
        }
        return open_ble_dev ();
    }

    int stop_stream_native (void *param)
    {
        if (!should_stop_stream)
        {
            should_stop_stream = true;
            read_characteristic_thread.join ();
        }
        int res = config_board_native ((char *)"s");
        std::queue<struct GanglionLibNative::GanglionDataNative> empty;
        std::swap (data_queue, empty); // free queue
        return res;
    }

    int start_stream_native (void *param)
    {
        int res = config_board_native ((char *)"b");
        if (res != (int)CustomExitCodesNative::STATUS_OK)
        {
            return res;
        }
        // from silicanlabs forum - write 0x00001 to enable notifications
        uint8 configuration[] = {0x01, 0x00};
        state = State::write_to_client_char;
        exit_code = (int)GanglionLibNative::SYNC_ERROR;
        ble_cmd_attclient_attribute_write (connection, client_char_handle, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        res = wait_for_callback (10);
        should_stop_stream = false;
        read_characteristic_thread = std::thread (read_characteristic_worker);
        return res;
    }

    int close_ganglion_native (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodesNative::GANGLION_IS_NOT_OPEN_ERROR;
        }
        state = State::close_called;

        if (!should_stop_stream)
        {
            stop_stream_native (NULL);
        }

        connection = -1;
        ganglion_handle_start = 0;
        ganglion_handle_end = 0;
        ganglion_handle_recv = 0;
        ganglion_handle_send = 0;

        uart_close ();

        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int get_data_native (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodesNative::GANGLION_IS_NOT_OPEN_ERROR;
        }
        state = State::get_data_called;
        if (data_queue.empty ())
        {
            return (int)CustomExitCodesNative::NO_DATA_ERROR;
        }
        struct GanglionDataNative *board_data = (struct GanglionDataNative *)param;
        struct GanglionDataNative data = data_queue.front ();
        board_data->timestamp = data.timestamp;
        for (int i = 0; i < 20; i++)
        {
            board_data->data[i] = data.data[i];
        }
        data_queue.pop ();
        return (int)CustomExitCodesNative::STATUS_OK;
    }

    int config_board_native (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodesNative::GANGLION_IS_NOT_OPEN_ERROR;
        }
        exit_code = (int)CustomExitCodesNative::SYNC_ERROR;
        char *config = (char *)param;
        int len = strlen (config);
        state = State::config_called;
        if (!ganglion_handle_send)
        {
            return (int)CustomExitCodesNative::SEND_CHARACTERISTIC_NOT_FOUND_ERROR;
        }
        ble_cmd_attclient_attribute_write (connection, ganglion_handle_send, len, (uint8 *)config);
        ble_cmd_attclient_execute_write (connection, 1);
        int res = wait_for_callback (15);
        return res;
    }

    int release_native (void *param)
    {
        if (initialized)
        {
            close_ganglion_native (NULL);
            state = State::none;
            initialized = false;
        }
        return (int)CustomExitCodesNative::STATUS_OK;
    }

} // GanglionLibNative
