#include "muse2.h"
#include "muse_constants.h"

#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

Muse2::Muse2 (struct BrainFlowInputParams params) : Board ((int)BoardIds::MUSE_2_BOARD, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

Muse2::~Muse2 ()
{
    skip_logs = true;
    release_session ();
}

int Muse2::prepare_session ()
{
    safe_logger (spdlog::level::trace, "prepare session");
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (bt.open_bt_adapter () != (int)BluetoothExitCodes::OK)
    {
        // Error codes could be processed better.
        // The current error code selections in this sourcefile could be considered stubs if
        // insufficient.
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    // find and connect to device

    if (params.mac_address.empty ())
    {
        std::cerr << "No mac address provided, scanning for a nearby muse ..." << std::endl;

        std::promise<std::string> device_found {};

        // TODO: check serial number if provided in params.  this can be queried for on connection
        // ("s" reply) if not available in scanning data.

        std::string muse_mac_prefix = "00:55:DA";
        std::string muse_name_prefix =
            "Muse"; // note the user can rename their devices, so this may be undesired

        auto callback = [&] (const char *address, const char *name) {
            if (muse_mac_prefix.compare (
                    0, muse_mac_prefix.size (), address, muse_mac_prefix.size ()) == 0 ||
                muse_name_prefix.compare (
                    0, muse_name_prefix.size (), name, muse_name_prefix.size ()) == 0)
            {
                try
                {
                    device_found.set_value (address);
                }
                catch (std::future_error const &)
                {
                    // already found a muse
                    return;
                }
                std::cerr << "Found " << name << std::endl;
            }
        };

        if (bt.start_bt_scanning (callback) != (int)BluetoothExitCodes::OK)
        {
            bt.close_bt_adapter ();
            return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
        }

        params.mac_address = device_found.get_future ().get ();

        bt.stop_bt_scanning ();
    }

    if (bt.connect_bt_dev (params.mac_address.c_str ()) != (int)BluetoothExitCodes::OK)
    {
        bt.close_bt_adapter ();
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }

    // connect to serial port on device

    bt.subscribe_bt_dev_characteristic (INTERAXON_GATT_SERVICE, MUSE_GATT_SERIAL, true,
        std::bind (&Muse2::serial_on_response, this, std::placeholders::_1, std::placeholders::_2));

    // configure device
    std::string preset_str;
    if (params.other_info.size () > 0)
    {
        if (params.other_info[0] != 'p')
        {
            preset_str = params.other_info;
        }
        else
        {
            preset_str = params.other_info.substr (1);
        }
    }
    else
    {
        preset_str = "63"; // this default preset enables all channels on the Muse S, including
                           // diagnostic ones.  untested on other muses.  on the original muse
                           // enabling all channels reduced the bit precision available in each
                           // channel.  feel free to change this default.
        std::cerr << "No muse preset set in other_info: defaulting to " << preset_str << std::endl;
    }

    try
    {
        json hwinfo = serial_write_throws ("v");
        json status = serial_write_throws ("s");

        // TODO: check serial number from hwinfo if specified in params
        // TODO: store featureset of device by parsing responses (2016, 2, S)

        std::cerr << "Muse version response: " << hwinfo.dump () << std::endl;
        std::cerr << "Muse status response: " << status.dump () << std::endl;

        // xloem's Muse S gives these version and status responses:
        //  {"ap":"headset","bl":"0.0.28","bn":61,"fi":"74058d3","hw":"00.7","rc":0,"sp":"Aster_revD"}
        //  {"bp":25,"hn":"MuseS-1C52","hs":"F006-4M2Z-3620","id":"0024003b 3152500e
        //  20383748","ma":"00-55-da-bb-1c-52","ps":81,"rc":0,"sn":"5007-CPYZ-1C52","ts":0}

        serial_write_throws ("p" + preset_str);
        if (serial_write_throws ("s")["ps"] != std::stol (preset_str, nullptr, 16))
        {
            // failed to set preset
            throw std::runtime_error (status.dump ());
        }
    }
    catch (...)
    {
        bt.close_bt_adapter ();
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Muse2::serial_on_response (const uint8_t *data, size_t len)
{

    // the muse serial data is formed of 20-byte data packets where each packet
    // has a 1-byte length, and then string data.

    // this author finds it worrying that uncleared memory containing leftover
    // internal state is sent over the wire after the string data.  the data is
    // truncated by the length in the first byte to remove the cruft data.
    // each reply is a json object, possibly sent via multiple packets.

    // given the muse also has no authentication process, and has a publicly
    // selectable preset that places it in a separate runstate possibly for
    // updating firmware, this author would not trust the muse with brainwaves
    // that need to be private or unaltered.

    try
    {
        std::string str (reinterpret_cast<const char *> (data), len);
        if (len % 20 != 0)
        {
            throw std::runtime_error (str);
        }

        // some drivers prefix new packets with copies of old ones.
        // maybe this is because the old ones haven't received their
        // notification reply yet?
        size_t offset = len - 20;
        size_t packetlen = str.at (offset);
        if (packetlen > 19)
        {
            throw std::runtime_error (str);
        }
        std::string packetstr = str.substr (offset + 1, packetlen);

        // std::cerr << "Serial reply data len=" << packetlen << "/" << len << ": " << packetstr
        //           << std::endl;

        std::unique_lock lk (serial_data_lock);
        serial_data_needslock += packetstr;

        if (packetstr[packetlen - 1] == '}')
        {
            std::string serial_data = std::move (serial_data_needslock);
            json result = json::parse (serial_data);

            if (result["rc"] != 0)
            {
                throw std::runtime_error (str);
            }
            serial_reply.set_value (result);
        }
    }
    catch (...)
    {
        try
        {
            serial_reply.set_exception (std::current_exception ());
        }
        catch (std::future_error const &)
        {
        }
    }
}

json Muse2::serial_write_throws (std::string cmd)
{
    serial_reply = std::promise<json> ();

    cmd.reserve (cmd.size () + 2);
    cmd.insert (cmd.begin (), cmd.size () + 1);
    cmd += '\n';

    if (bt.write_bt_dev_characteristic (INTERAXON_GATT_SERVICE, MUSE_GATT_SERIAL, false,
            reinterpret_cast<const uint8_t *> (cmd.data ()),
            cmd.size ()) != (int)BluetoothExitCodes::OK)
    {
        throw std::runtime_error ("failed to write characteristic");
    }

    return serial_reply.get_future ().get ();
}

int Muse2::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    // keep_alive = true;
    // STUB
    // streaming_thread = std::thread ([this] { this->read_thread (); });
    // is_streaming = true;
    // return (int)BrainFlowExitCodes::STATUS_OK;
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Muse2::stop_stream ()
{
    safe_logger (spdlog::level::trace, "stop stream");
    // STUB
    // if (is_streaming)
    // {
    //     keep_alive = false;
    //     is_streaming = false;
    //     streaming_thread.join ();
    //     return (int)BrainFlowExitCodes::STATUS_OK;
    // }
    //  else
    // {
    //     return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    // }
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Muse2::release_session ()
{
    safe_logger (spdlog::level::trace, "release session");
    if (initialized)
    {
        stop_stream ();
        free_packages ();

        bt.disconnect_bt_dev ();
        bt.close_bt_adapter ();

        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Muse2::read_thread ()
{
    // STUB
    // unsigned char counter = 0;
    // std::vector<int> exg_channels = board_descr["eeg_channels"]; // same
    // channels for eeg\emg\ecg
    // double *sin_phase_rad = new double[exg_channels.size ()];
    // for (unsigned int i = 0; i < board_descr["eeg_channels"].size (); i++)
    // {
    //     sin_phase_rad[i] = 0.0;
    // }
    // int sampling_rate = board_descr["sampling_rate"];
    // int initial_sleep_time = 1000 / sampling_rate;
    // int sleep_time = initial_sleep_time;
    // std::uniform_real_distribution<double> dist_around_one (0.90, 1.10);
    // uint64_t seed = std::chrono::high_resolution_clock::now ().time_since_epoch
    // ().count ();
    // std::mt19937 mt (static_cast<uint32_t> (seed));
    // double accumulated_time_delta = 0.0;

    // int num_rows = board_descr["num_rows"];
    // double *package = new double[num_rows];
    // for (int i = 0; i < num_rows; i++)
    // {
    //     package[i] = 0.0;
    // }

    while (keep_alive)
    {
        //    auto start = std::chrono::high_resolution_clock::now ();
        //    package[board_descr["package_num_channel"].get<int> ()] =
        // (double)counter;
        //    for (unsigned int i = 0; i < exg_channels.size (); i++)
        //    {
        //        double amplitude = 10.0 * (i + 1);
        //        double noise = 0.1 * (i + 1);
        //        double freq = 5.0 * (i + 1);
        //        double shift = 0.05 * i;
        //        double range = (amplitude * noise) / 2.0;
        //        std::uniform_real_distribution<double> dist (0 - range, range);
        //        sin_phase_rad[i] += 2.0f * M_PI * freq / (double)sampling_rate;
        //        if (sin_phase_rad[i] > 2.0f * M_PI)
        //        {
        //            sin_phase_rad[i] -= 2.0f * M_PI;
        //        }
        //        package[exg_channels[i]] =
        //            (amplitude + dist (mt)) * sqrt (2.0) * sin (sin_phase_rad[i] +
        // shift);
        //    }
        //    for (int channel : board_descr["accel_channels"])
        //    {
        //        package[channel] = dist_around_one (mt) - 0.1;
        //    }
        //    for (int channel : board_descr["gyro_channels"])
        //    {
        //        package[channel] = dist_around_one (mt) - 0.1;
        //    }
        //    for (int channel : board_descr["eda_channels"])
        //    {
        //        package[channel] = dist_around_one (mt);
        //    }
        //    for (int channel : board_descr["ppg_channels"])
        //    {
        //        package[channel] = 5000.0 * dist_around_one (mt);
        //    }
        //    for (int channel : board_descr["temperature_channels"])
        //    {
        //        package[channel] = dist_around_one (mt) / 10.0 + 36.5;
        //    }
        //    for (int channel : board_descr["resistance_channels"])
        //    {
        //        package[channel] = 1000.0 * dist_around_one (mt);
        //    }
        //    package[board_descr["battery_channel"].get<int> ()] = (dist_around_one
        // (mt) - 0.1) * 100;
        //    package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp
        // ();

        //    push_package (package); // use this method to submit data to buffers

        //    counter++;
        //    if (sleep_time > 0)
        //    {
        // #ifdef _WIN32
        //        Sleep (sleep_time);
        // #else
        //        usleep (sleep_time * 1000);
        // #endif
        //    }

        //    auto stop = std::chrono::high_resolution_clock::now ();
        //    auto duration =
        //        std::chrono::duration_cast<std::chrono::microseconds> (stop -
        // start).count ();
        //    accumulated_time_delta += duration - initial_sleep_time * 1000;
        //    sleep_time = initial_sleep_time - (int)(accumulated_time_delta /
        // 1000.0);
        //    accumulated_time_delta =
        //        accumulated_time_delta - 1000.0 * (int)(accumulated_time_delta /
        // 1000.0);
    }
    // delete[] sin_phase_rad;
    // delete[] package;
}

int Muse2::config_board (std::string config, std::string &response)
{
    // STUB
    // return (int)BrainFlowExitCodes::STATUS_OK;
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
