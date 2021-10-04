#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "ironbci.h"
#include "serial.h"
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
        safe_logger (spdlog::level::err, "Serial port is empty");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    spi = spi_new ();
    gpio_in = gpio_new ();
    if (spi_open (spi, params.serial_port.c_str (), 0b01, 1000000) < 0)
    {
        spi_free (spi);
        spi = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    if (gpio_open (gpio_in, "/dev/gpiochip0", 26, GPIO_DIR_IN) < 0)
    {
        gpio_free (gpio_in);
        gpio_in = NULL;
        spi_free (spi);
        spi = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }


    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::start_stream (int buffer_size, char *streamer_params)
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

    int spi_res = 0;
    // led
    uint8_t adress = 0x40 | 0x14;
    uint8_t buf[3] = {adress, 0x00, 0x80};
    spi_res = spi_transfer (spi, buf, buf, 3);
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::error, "failed to set led: {}", spi_res);
    }
    else
    {
        // config 1
        uint8_t adress1 = 0x40 | 0x01;
        uint8_t buf1[3] = {adress1, 0x00, 0x96};
        spi_res = spi_transfer (spi, buf1, buf1, 3);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::error, "failed to set config1: {}", spi_res);
    }
    else
    {
        // config 2
        uint8_t adress2 = 0x40 | 0x02;
        uint8_t buf2[3] = {adress2, 0x00, 0xD4};
        spi_res = spi_transfer (spi, buf2, buf2, 3);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::error, "failed to set config2: {}", spi_res);
    }
    else
    {
        // config 3
        uint8_t adress3 = 0x40 | 0x03;
        uint8_t buf3[3] = {adress3, 0x00, 0xEC};
        spi_res = spi_transfer (spi, buf3, buf3, 3);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::error, "failed to set config3: {}", spi_res);
    }
    else
    {
        // misc1
        uint8_t adress4 = 0x40 | 0x15;
        uint8_t buf4[3] = {adress4, 0x00, 0x20};
        spi_res = spi_transfer (spi, buf4, buf4, 3);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::error, "failed to set misc1: {}", spi_res);
    }
    else
    {
        // send command 1
        uint8_t buf5[1] = {0x10};
        spi_res = spi_transfer (spi, buf5, buf5, 1);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::error, "failed to set send command1: {}", spi_res);
    }
    else
    {
        // send command 2
        uint8_t buf6[1] = {0x08};
        spi_res = spi_transfer (spi, buf6, buf6, 1);
    }
    if (spi_res != 0)
    {
        safe_logger (spdlog::level::error, "failed to set send command2: {}", spi_res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
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
        // todo send smth to stop streaming?
        return (int)BrainFlowExitCodes::STATUS_OK;
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
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    bool was_one = false;
    uint32_t data_test = 0x7FFFFF;
    uint32_t data_check = 0xFFFFFF;
    double timestamp = 0;

    while (keep_alive)
    {
        bool value;
        int res = gpio_read (gpio_in, &value);
        if (res != 0)
        {
            safe_logger (spdlog::level::warn, "failed to read from gpio: {}", res);
            continue;
        }

        if (value == 1)
        {
            timestamp = get_timestamp ();
            was_one = true;
            continue;
        }

        if ((value == 0) && (was_one))
        {
            was_one = false;
            res = spi_transfer (spi, buf, buf, 27);
            if (res != 0)
            {
                safe_logger (spdlog::level::warn, "failed to read from spi: {}", res);
                continue;
            }
            for (size_t i = 0; i < eeg_channels.size (); i++)
            {
                size_t offset = 3 + 3 * i;
                uint32_t voltage = (buf[offset] << 8) | buf[offset + 1];
                voltage = (voltage << 8) | buf[offset + 2];
                voltage = voltage | data_test;
                if (voltage == data_check)
                {
                    voltage = 16777214 - voltage;
                }
                package[eeg_channels[i]] = voltage;
            }
            package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
            push_package (package);
        }
    }
    delete[] package;
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

int IronBCI::start_stream (int buffer_size, char *streamer_params)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
