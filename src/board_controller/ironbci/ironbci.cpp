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

    spi = spi_new ();
    gpio_in = gpio_new ();
    if (spi_open (spi, params.serial_port.c_str (), 0b01, 100000) < 0)
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

    uint8_t wakeup[1] = {0x02};
    int spi_res = spi_transfer (spi, wakeup, wakeup, 1);
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to wakeup: {}", spi_res);
    }
    else
    {
        uint8_t buf_reset[1] = {0x06};
        spi_res = spi_transfer (spi, buf_reset, buf_reset, 1);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to reset: {}", spi_res);
    }
    else
    {
        uint8_t sdatac[1] = {0x11};
        spi_res = spi_transfer (spi, sdatac, sdatac, 1);
    }

    if (spi_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to sdatac: {}", spi_res);
        gpio_free (gpio_in);
        gpio_in = NULL;
        spi_free (spi);
        spi = NULL;
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    else
    {
        int gpio_res = gpio_set_edge (gpio_in, GPIO_EDGE_FALLING);
        if (gpio_res != 0)
        {
            safe_logger (spdlog::level::err, "Failed to set gpio edge event: {}", gpio_res);
            gpio_free (gpio_in);
            gpio_in = NULL;
            spi_free (spi);
            spi = NULL;
            return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }
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

    int spi_res = 0;
    // led
    uint8_t adress = 0x40 | 0x14;
    uint8_t buf[3] = {adress, 0x00, 0x80};
    spi_res = spi_transfer (spi, buf, buf, 3);
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to set led: {}", spi_res);
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
        safe_logger (spdlog::level::err, "failed to set config1: {}", spi_res);
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
        safe_logger (spdlog::level::err, "failed to set config2: {}", spi_res);
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
        safe_logger (spdlog::level::err, "failed to set config3: {}", spi_res);
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
        safe_logger (spdlog::level::err, "failed to set misc1: {}", spi_res);
    }
    else
    {
        // send command 1
        uint8_t buf5[1] = {0x10};
        spi_res = spi_transfer (spi, buf5, buf5, 1);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to set send command1: {}", spi_res);
    }
    else
    {
        // send command 2
        uint8_t buf6[1] = {0x08};
        spi_res = spi_transfer (spi, buf6, buf6, 1);
    }
    if (spi_res < 0)
    {
        safe_logger (spdlog::level::err, "failed to set send command2: {}", spi_res);
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
        uint8_t stop[1] = {0x0A};
        if (spi_transfer (spi, stop, stop, 1) < 0)
        {
            return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }
        else
        {
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
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

    double eeg_scale = 4.5 / float ((pow (2, 23) - 1)) / 8 * 1000000.;
    double timestamp = 0;
    int counter = 0;
    int timeout_ms = 1000;

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
            gpio_edge_t edge_type = GPIO_EDGE_NONE;
            gpio_res = gpio_read_event (gpio_in, &edge, NULL);
            if (gpio_res != 0)
            {
                safe_logger (spdlog::level::warn, "failed to get gpio event: {}", gpio_res);
                continue;
            }
            if (edge != GPIO_EDGE_FALLING)
            {
                continue;
            }
            int spi_res = spi_transfer (spi, buf, buf, 27);
            if (spi_res != 0)
            {
                safe_logger (spdlog::level::warn, "failed to read from spi: {}", spi_res);
                continue;
            }
            for (size_t i = 0; i < eeg_channels.size (); i++)
            {
                package[eeg_channels[i]] =
                    (double)eeg_scale * cast_24bit_to_int32 (buf + 3 + 3 * i);
            }
            package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
            package[board_descr["package_num_channel"].get<int> ()] = counter++;
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

int IronBCI::start_stream (int buffer_size, const char *streamer_params)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
