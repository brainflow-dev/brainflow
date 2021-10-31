#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "ironbci.h"
#include "math.h"
#include "timestamp.h"


#ifdef USE_PERIPHERY
IronBCI::IronBCI (struct BrainFlowInputParams params) : Board ((int)BoardIds::IRONBCI_BOARD, params)
{
    spi = NULL;
    gpio_in = NULL;
    keep_alive = false;
    initialized = false;
}

IronBCI::~IronBCI ()
{
    skip_logs = true;
    release_session ();
}

int IronBCI::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.serial_port.empty ())
    {
        params.serial_port = "/dev/spidev0.0";
        safe_logger (spdlog::level::info, "Use serial port {}", params.serial_port.c_str ());
    }

    gpio_in = gpio_new ();
    if (gpio_open (gpio_in, "/dev/gpiochip0", 26, GPIO_DIR_IN) < 0)
    {
        safe_logger (spdlog::level::err, "failed to open gpio");
        gpio_free (gpio_in);
        gpio_in = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    spi = spi_new ();
    if (spi_open_advanced (spi, params.serial_port.c_str (), 0b01, 1000000, MSB_FIRST, 8, 1) < 0)
    {
        safe_logger (spdlog::level::err, "failed to open spi dev");
        spi_free (spi);
        spi = NULL;
        gpio_free (gpio_in);
        gpio_in = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    int gpio_res = gpio_set_edge (gpio_in, GPIO_EDGE_FALLING);
    if (gpio_res != 0)
    {
        safe_logger (spdlog::level::err, "failed to set gpio edge event handler: {}", gpio_res);
        spi_free (spi);
        spi = NULL;
        gpio_free (gpio_in);
        gpio_in = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::start_stream (int buffer_size, const char *streamer_params)
{
    if (keep_alive)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    int spi_res = write_reg (0x14, 0x80); // led
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x05, 0x00); // ch1
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x06, 0x0); // ch2
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x07, 0x00); // ch3
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x08, 0x00); // ch4
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x09, 0x00); // ch5
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x0A, 0x00); // ch6
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x0B, 0x00); // ch7
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x0C, 0x00); // ch8
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x15, 0x20); // mics
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x01, 0x96); // reg1
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x02, 0xD4); // reg2
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = write_reg (0x03, 0xE0); // reg3
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = send_command (0x10); // sdatc
    }
    if (spi_res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        spi_res = send_command (0x08); // start
    }

    if (spi_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return spi_res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }
        return send_command (0x0A); // stop
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int IronBCI::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
    }
    if (spi)
    {
        spi_close (spi);
        spi_free (spi);
        spi = NULL;
    }
    if (gpio_in)
    {
        gpio_close (gpio_in);
        gpio_free (gpio_in);
        gpio_in = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::config_board (std::string config, std::string &response)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

void IronBCI::read_thread ()
{
    uint8_t buf[27] = {0};
    uint8_t zero27[27] = {0};
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    double eeg_scale = 4.5 / float ((pow (2, 23) - 1)) / 8 * 1000000.;
    double timestamp = 0;
    int counter = 0;
    int timeout_ms = 1000;
    uint32_t data_test = 0x7FFFFF;
    uint32_t data_check = 0xFFFFFF;

    while (keep_alive)
    {
        int gpio_res = gpio_poll (gpio_in, timeout_ms);
        timestamp = get_timestamp ();
        if (gpio_res == 0)
        {
            safe_logger (spdlog::level::trace, "no gpio event in {} ms", timeout_ms);
        }
        else if (gpio_res < 0)
        {
            safe_logger (spdlog::level::warn, "error in gpio_poll: {}", gpio_res);
        }
        else
        {
            gpio_edge_t edge = GPIO_EDGE_NONE;
            gpio_res = gpio_read_event (gpio_in, &edge, NULL);
            if (gpio_res != 0)
            {
                safe_logger (spdlog::level::warn, "failed to get gpio event: {}", gpio_res);
                continue;
            }
            if (edge != GPIO_EDGE_FALLING)
            {
                safe_logger (spdlog::level::warn, "unexpected gpio event");
                continue;
            }
            int spi_res = spi_transfer (spi, zero27, buf, 27);
            if (spi_res != 0)
            {
                safe_logger (spdlog::level::warn, "failed to read from spi: {}", spi_res);
                continue;
            }
            for (size_t i = 0; i < eeg_channels.size (); i++)
            {
                int offset = 3 * i + 3;
                uint32_t voltage = (buf[offset] << 8) | buf[offset + 1];
                voltage = (voltage << 8) | buf[offset + 2];
                uint32_t voltage_test = voltage | data_test;
                if (voltage_test == data_check)
                {
                    voltage = 16777214 - voltage;
                }

                package[eeg_channels[i]] = 0.27 * voltage;
            }
            package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
            package[board_descr["package_num_channel"].get<int> ()] = counter++;
            push_package (package);
        }
    }
    delete[] package;
}

int IronBCI::write_reg (uint8_t reg_address, uint8_t val)
{
    uint8_t zero3[3] = {0, 0, 0};
    uint8_t reg_address_shift = 0x40 | reg_address;
    uint8_t write[3] = {reg_address_shift, 0x00, val};
    int spi_res = spi_transfer (spi, write, zero3, 3);
    if (spi_res < 0)
    {
        safe_logger (
            spdlog::level::err, "failed to write reg {}, error: {}", (int)reg_address, spi_res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::send_command (uint8_t command)
{
    uint8_t zero = 0;
    int spi_res = spi_transfer (spi, &command, &zero, 1);
    if (spi_res < 0)
    {
        safe_logger (
            spdlog::level::err, "failed to write command {}, error: {}", (int)command, spi_res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

#else

IronBCI::IronBCI (struct BrainFlowInputParams params) : Board ((int)BoardIds::IRONBCI_BOARD, params)
{
}

IronBCI::~IronBCI ()
{
}

int IronBCI::prepare_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::release_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::stop_stream ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::start_stream (int buffer_size, const char *streamer_params)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
