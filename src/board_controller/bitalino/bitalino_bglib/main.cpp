#include <chrono>
#include <ctype.h>
#include <deque>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "cmd_def.h"
#include "helpers.h"
#include "uart.h"

#include "ticket_lock.h"

#include "bitalino_functions.h"
#include "bitalino_types.h"

// read Bluetooth_Smart_Software_v1.3.1_API_Reference.pdf to understand this code

namespace BitalinoLib
{
    volatile int exit_code = (int)BitalinoLib::SYNC_ERROR;
    char uart_port[1024];
    int timeout = 15;

    std::deque<struct BitalinoLib::BitalinoData> data_queue;
    TicketLock lock;
    volatile bd_addr connect_addr;

    volatile uint8 connection = -1;
    volatile uint16 bitalino_handle_start = 0;
    volatile uint16 bitalino_handle_end = 0;
    volatile uint16 bitalino_handle_recv = 0;
    volatile uint16 bitalino_handle_send = 0;
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
            struct BitalinoInputData *input = (struct BitalinoInputData *)param;
            strcpy (uart_port, input->uart_port);
            timeout = input->timeout;
            bglib_output = output;
            exit_code = (int)CustomExitCodes::SYNC_ERROR;
            initialized = true;
        }
        return (int)CustomExitCodes::STATUS_OK;
    }

    int open_bitalino (void *param)
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

        res = wait_for_callback (timeout);
        if (res != (int)CustomExitCodes::STATUS_OK)
        {
            return res;
        }
        ble_cmd_gap_end_procedure ();
        return open_ble_dev ();
    }

    int open_bitalino_mac_addr (void *param)
    {
        if (uart_open (uart_port))
        {
            return (int)CustomExitCodes::BITALINO_NOT_FOUND_ERROR;
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
            return (int)CustomExitCodes::INVALID_MAC_ADDR_ERROR;
        }
        return open_ble_dev ();
    }

#ifdef __linux__
    int stop_stream (void *param)
    {
        // dirty hack to solve https://github.com/brainflow-dev/brainflow/issues/24
        // for Ubuntu callbacks for config_board are not triggered often if streaming is running.
        // Assumption - maybe serial buffer size is smaller in Ubuntu and notification messages fill
        // entire buffer. Solution for this assumption - call ble_cmd_attclient_attribute_write all
        // the time in the thread instead single invocation.
        if (!initialized)
        {
            return (int)CustomExitCodes::BITALINO_IS_NOT_OPEN_ERROR;
        }
        if (!should_stop_stream)
        {
            should_stop_stream = true;
            read_characteristic_thread.join ();
        }
        exit_code = (int)CustomExitCodes::SYNC_ERROR;
        state = State::CONFIG_CALLED;
        if (!bitalino_handle_send)
        {
            return (int)CustomExitCodes::SEND_CHARACTERISTIC_NOT_FOUND_ERROR;
        }
        volatile bool stop_config_thread = false;
        std::thread config_thread = std::thread ([&] () {
            while (!stop_config_thread)
            {
                ble_cmd_attclient_attribute_write (
                    connection, bitalino_handle_send, 1, (uint8 *)"\0");
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
        uint8 stop_command = 0;
        int res = config_board (&stop_command, 1);
        data_queue.clear ();
        return res;
    }
#endif

    int start_stream (void *param)
    {
        // https://bitalino.com/storage/uploads/media/revolution-mcu-block-datasheet.pdf
        uint8 start_command = 253; // all channels on, live mode
        int res = config_board (&start_command, 1);
        if (res != (int)CustomExitCodes::STATUS_OK)
        {
            return res;
        }
        // from silicanlabs forum - write 0x00001 to enable notifications
        uint8 configuration[] = {0x01, 0x00};
        state = State::WRITE_TO_CLIENT_CHAR;
        exit_code = (int)BitalinoLib::SYNC_ERROR;
        ble_cmd_attclient_attribute_write (connection, client_char_handle, 2, &configuration);
        res = wait_for_callback (timeout);

        if (res == (int)CustomExitCodes::STATUS_OK)
        {
            should_stop_stream = false;
            read_characteristic_thread = std::thread (read_characteristic_worker);
        }

        return res;
    }

    int close_bitalino (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodes::BITALINO_IS_NOT_OPEN_ERROR;
        }
        state = State::CLOSE_CALLED;

        if (!should_stop_stream)
        {
            stop_stream (NULL);
        }

        connection = -1;
        bitalino_handle_start = 0;
        bitalino_handle_end = 0;
        bitalino_handle_recv = 0;
        bitalino_handle_send = 0;

        uart_close ();

        return (int)CustomExitCodes::STATUS_OK;
    }

    int get_data (void *param)
    {
        if (!initialized)
        {
            return (int)CustomExitCodes::BITALINO_IS_NOT_OPEN_ERROR;
        }
        if (should_stop_stream)
        {
            return (int)CustomExitCodes::NO_DATA_ERROR;
        }
        state = State::GET_DATA_CALLED;
        int res = (int)CustomExitCodes::STATUS_OK;
        lock.lock ();
        if (data_queue.empty ())
        {
            res = (int)CustomExitCodes::NO_DATA_ERROR;
        }
        else
        {
            try
            {
                struct BitalinoData *board_data = (struct BitalinoData *)param;
                struct BitalinoData data = data_queue.at (
                    0); // at ensures out of range exception, front has undefined behavior
                for (int i = 0; i < BitalinoData::SIZE; i++)
                {
                    board_data->data[i] = data.data[i];
                }
                data_queue.pop_front ();
            }
            catch (...)
            {
                res = (int)CustomExitCodes::NO_DATA_ERROR;
            }
        }
        lock.unlock ();
        return res;
    }

    int release (void *param)
    {
        if (initialized)
        {
            close_bitalino (NULL);
            state = State::NONE;
            initialized = false;
            data_queue.clear ();
        }
        return (int)CustomExitCodes::STATUS_OK;
    }

} // BitalinoLib
