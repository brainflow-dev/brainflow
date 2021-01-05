#include <chrono>
#include <condition_variable>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "helpers.h"
#include "uart.h"

#include "ganglion_types.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FIRST_HANDLE 0x0001
#define LAST_HANDLE 0xffff


namespace GanglionLib
{
    extern volatile int exit_code;
    extern int timeout;
    extern volatile bd_addr connect_addr;
    extern volatile uint8 connection;
    extern volatile uint16 client_char_handle;
    extern char uart_port[1024];
    extern volatile State state;
    extern std::condition_variable cv;

    void output (uint8 len1, uint8 *data1, uint16 len2, uint8 *data2)
    {
        if (uart_tx (len1, data1) || uart_tx (len2, data2))
        {
            exit_code = (int)GanglionLib::PORT_OPEN_ERROR;
        }
    }

    // reads messages and calls required callbacks (copypaste from sample)
    int read_message (int timeout_ms)
    {
        unsigned char *data = NULL;
        struct ble_header hdr;
        int r;

        r = uart_rx (sizeof (hdr), (unsigned char *)&hdr, timeout_ms);
        if (!r)
        {
            return -1; // timeout
        }
        else if (r < 0)
        {
            exit_code = (int)GanglionLib::PORT_OPEN_ERROR;
            return 1; // fails to read
        }
        if (hdr.lolen)
        {
            data = new unsigned char[hdr.lolen];
            r = uart_rx (hdr.lolen, data, UART_TIMEOUT);
            if (r <= 0)
            {
                exit_code = (int)GanglionLib::PORT_OPEN_ERROR;
                delete[] data;
                return 1; // fails to read
            }
        }

        const struct ble_msg *msg = ble_get_msg_hdr (hdr);
        if (msg)
        {
            msg->handler (data);
        }
        delete[] data;
        return 0;
    }

    int open_ble_dev ()
    {
        exit_code = (int)GanglionLib::SYNC_ERROR;
        // send command to connect
        state = State::INITIAL_CONNECTION;
        // changing values here leads to package loss, dont touch it
        ble_cmd_gap_connect_direct (&connect_addr, gap_address_type_random, 10, 76, 100, 0);
        int res = wait_for_callback (timeout);
        if (res != (int)GanglionLib::STATUS_OK)
        {
            return res;
        }
        state = State::OPEN_CALLED;

        exit_code = (int)GanglionLib::SYNC_ERROR;
        uint8 primary_service_uuid[] = {0x00, 0x28};
        ble_cmd_attclient_read_by_group_type (
            connection, FIRST_HANDLE, LAST_HANDLE, 2, primary_service_uuid);

        res = wait_for_callback (timeout);
        if (res != (int)GanglionLib::STATUS_OK)
        {
            return res;
        }

        // from siliconlabs forum - write 0x00001 to enable notifications
        // copypasted in start_stream method but lets keep it in 2 places
        uint8 configuration[] = {0x01, 0x00};
        state = State::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)GanglionLib::SYNC_ERROR;
        ble_cmd_attclient_attribute_write (connection, client_char_handle, 2, &configuration);
        ble_cmd_attclient_execute_write (connection, 1);
        return wait_for_callback (timeout);
    }

    int wait_for_callback (int num_seconds)
    {
        auto start_time = std::chrono::high_resolution_clock::now ();
        int run_time = 0;
        while ((run_time < num_seconds) && (exit_code == (int)GanglionLib::SYNC_ERROR))
        {
            if (read_message (UART_TIMEOUT) > 0)
            {
                break;
            }
            auto end_time = std::chrono::high_resolution_clock::now ();
            run_time = (int)std::chrono::duration_cast<std::chrono::seconds> (end_time - start_time)
                           .count ();
        }
        return exit_code;
    }

    int reset_ble_dev ()
    {
        // Reset dongle to get it into known state
        ble_cmd_system_reset (0);
        uart_close ();
        int i;
        for (i = 0; i < 5; i++)
        {
#ifdef _WIN32
            Sleep (500);
#else
            usleep (500000);
#endif
            if (!uart_open (uart_port))
            {
                break;
            }
        }
        if (i == 5)
        {
            return (int)GanglionLib::PORT_OPEN_ERROR;
        }
        return (int)GanglionLib::STATUS_OK;
    }
}