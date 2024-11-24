#include "emotibit.h"

#include <chrono>
#include <sstream>
#include <stdint.h>
#include <string.h>

#include "broadcast_server.h"
#include "custom_cast.h"
#include "json.hpp"
#include "network_interfaces.h"
#include "timestamp.h"

#include "emotibit_defines.h"

using json = nlohmann::json;


Emotibit::Emotibit (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::EMOTIBIT_BOARD, params)
{
    data_socket = NULL;
    control_socket = NULL;
    adv_socket = NULL;
    keep_alive = false;
    initialized = false;
    control_port = -1;
    data_port = -1;
}

Emotibit::~Emotibit ()
{
    skip_logs = true;
    release_session ();
}

int Emotibit::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if (params.timeout < 2)
    {
        params.timeout = 4;
    }

    int res = create_adv_connection ();
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = create_data_connection ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = create_control_connection ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = send_connect_msg ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = wait_for_connection ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = send_control_msg (MODE_LOW_POWER);
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (adv_socket != NULL)
        {
            delete adv_socket;
            adv_socket = NULL;
        }
        if (data_socket != NULL)
        {
            delete data_socket;
            data_socket = NULL;
        }
        if (control_socket != NULL)
        {
            delete control_socket;
            control_socket = NULL;
        }
    }
    else
    {
        initialized = true;
        connection_thread = std::thread ([this] { this->ping_thread (); });
    }

    return res;
}

int Emotibit::config_board (std::string conf, std::string &response)
{
    return send_control_msg (conf.c_str ());
}

int Emotibit::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
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
    res = send_control_msg (MODE_NORMAL_POWER);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        keep_alive = true;
        streaming_thread = std::thread ([this] { this->read_thread (); });
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Emotibit::stop_stream ()
{
    if (keep_alive)
    {
        if (send_control_msg (MODE_LOW_POWER) != (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "failed to set low power mode");
        }
        keep_alive = false;
        streaming_thread.join ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
}

int Emotibit::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        initialized = false;
        connection_thread.join ();
        free_packages ();
        send_control_msg (EMOTIBIT_DISCONNECT);
        if (data_socket)
        {
            data_socket->close ();
            delete data_socket;
            data_socket = NULL;
        }
        if (control_socket)
        {
            control_socket->close ();
            delete control_socket;
            control_socket = NULL;
        }
        if (adv_socket)
        {
            adv_socket->close ();
            delete adv_socket;
            adv_socket = NULL;
        }
        control_port = -1;
        data_port = -1;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Emotibit::read_thread ()
{
    constexpr int max_size = 32768;
    char message[max_size];

    // emotibit sends multiple data points per transaction and for example accelerometer x and y
    // data are in different transactions, we have to align them and use max_datapoints_in_package
    // arrays instead single one
    constexpr int max_datapoints_in_package =
        1024; // experimental(random) value, in practice I saw max 9
    double *default_packages[max_datapoints_in_package];
    double *aux_packages[max_datapoints_in_package];
    double *anc_packages[max_datapoints_in_package];
    int num_default_rows = board_descr["default"]["num_rows"];
    int num_aux_rows = board_descr["auxiliary"]["num_rows"];
    int num_anc_rows = board_descr["ancillary"]["num_rows"];

    for (int cur_package = 0; cur_package < max_datapoints_in_package; cur_package++)
    {
        default_packages[cur_package] = new double[num_default_rows];
        for (int i = 0; i < num_default_rows; i++)
        {
            default_packages[cur_package][i] = 0.0;
        }
        aux_packages[cur_package] = new double[num_aux_rows];
        for (int i = 0; i < num_aux_rows; i++)
        {
            aux_packages[cur_package][i] = 0.0;
        }
        anc_packages[cur_package] = new double[num_anc_rows];
        for (int i = 0; i < num_anc_rows; i++)
        {
            anc_packages[cur_package][i] = 0.0;
        }
    }

    while (keep_alive)
    {
        int bytes_recv = data_socket->recv (message, max_size);
        if (bytes_recv < 1)
        {
            safe_logger (spdlog::level::trace, "no data received");
            continue;
        }
        std::string message_received = std::string (message, bytes_recv);
        std::vector<std::string> splitted_packages =
            split_string (message_received, PACKET_DELIMITER_CSV);
        for (std::string recv_package : splitted_packages)
        {
            int package_num = 0;
            int data_len = 0;
            std::string type_tag = "";
            if (get_header (recv_package, &package_num, &data_len, type_tag))
            {
                // default package
                std::vector<std::string> payload = get_payload (recv_package, data_len);
                int channel = -1;
                if (type_tag == ACCELEROMETER_X)
                {
                    channel = board_descr["default"]["accel_channels"][0];
                }
                if (type_tag == ACCELEROMETER_Y)
                {
                    channel = board_descr["default"]["accel_channels"][1];
                }
                if (type_tag == ACCELEROMETER_Z)
                {
                    channel = board_descr["default"]["accel_channels"][2];
                }
                if (type_tag == GYROSCOPE_X)
                {
                    channel = board_descr["default"]["gyro_channels"][0];
                }
                if (type_tag == GYROSCOPE_Y)
                {
                    channel = board_descr["default"]["gyro_channels"][1];
                }
                if (type_tag == GYROSCOPE_Z)
                {
                    channel = board_descr["default"]["gyro_channels"][2];
                }
                if (type_tag == MAGNETOMETER_X)
                {
                    channel = board_descr["default"]["magnetometer_channels"][0];
                }
                if (type_tag == MAGNETOMETER_Y)
                {
                    channel = board_descr["default"]["magnetometer_channels"][1];
                }
                if (type_tag == MAGNETOMETER_Z)
                {
                    channel = board_descr["default"]["magnetometer_channels"][2];
                }
                if (channel > 0)
                {
                    for (int i = 0; i < std::min ((int)payload.size (), max_datapoints_in_package);
                         i++)
                    {
                        default_packages[i]
                                        [board_descr["default"]["timestamp_channel"].get<int> ()] =
                                            get_timestamp ();
                        default_packages[i][board_descr["default"]["package_num_channel"]
                                                .get<int> ()] = package_num;
                        try
                        {
                            default_packages[i][channel] = std::stod (payload[i]);
                        }
                        catch (...)
                        {
                            safe_logger (spdlog::level::warn, "invalid data in payload: {}",
                                payload[i].c_str ());
                        }
                    }
                    channel = -1;
                }
                // push default preset when magnetometer z is received
                if (type_tag == MAGNETOMETER_Z)
                {
                    for (int i = 0; i < data_len; i++)
                    {
                        push_package (default_packages[i], (int)BrainFlowPresets::DEFAULT_PRESET);
                    }
                }
                // auxuliary package
                if (type_tag == PPG_INFRARED)
                {
                    channel = board_descr["auxiliary"]["ppg_channels"][0];
                }
                if (type_tag == PPG_RED)
                {
                    channel = board_descr["auxiliary"]["ppg_channels"][1];
                }
                if (type_tag == PPG_GREEN)
                {
                    channel = board_descr["auxiliary"]["ppg_channels"][2];
                }
                if (channel > 0)
                {
                    for (int i = 0; i < std::min ((int)payload.size (), max_datapoints_in_package);
                         i++)
                    {
                        aux_packages[i][board_descr["auxiliary"]["timestamp_channel"].get<int> ()] =
                            get_timestamp ();
                        aux_packages[i]
                                    [board_descr["auxiliary"]["package_num_channel"].get<int> ()] =
                                        package_num;
                        try
                        {
                            aux_packages[i][channel] = std::stod (payload[i]);
                        }
                        catch (...)
                        {
                            safe_logger (spdlog::level::warn, "invalid data in payload: {}",
                                payload[i].c_str ());
                        }
                    }
                    channel = -1;
                }
                // push aux preset when ppg green is received
                if (type_tag == PPG_GREEN)
                {
                    for (int i = 0; i < data_len; i++)
                    {
                        push_package (aux_packages[i], (int)BrainFlowPresets::AUXILIARY_PRESET);
                    }
                }
                // ancillary package
                if ((type_tag == TEMPERATURE_1) || (type_tag == THERMOPILE))
                {
                    int temperature_channel = 0;
                    if (type_tag == TEMPERATURE_1)
                    {
                        temperature_channel = board_descr["ancillary"]["temperature_channels"][0];
                    }
                    if (type_tag == THERMOPILE)
                    {
                        temperature_channel = board_descr["ancillary"]["other_channels"][0];
                    }
                    // upsample temperature data 2x to match eda
                    if (payload.size () < max_datapoints_in_package / 2)
                    {
                        for (int i = 0; i < (int)payload.size (); i++)
                        {
                            try
                            {
                                anc_packages[i * 2][temperature_channel] = std::stod (payload[i]);
                                anc_packages[i + 1][temperature_channel] =
                                    anc_packages[i][temperature_channel];
                            }
                            catch (...)
                            {
                                safe_logger (spdlog::level::warn, "invalid data in payload: {}",
                                    payload[i].c_str ());
                            }
                        }
                    }
                    // should not happen, no place in buffer for upsampling, keep as is
                    else
                    {
                        for (int i = 0;
                             i < std::min ((int)payload.size (), max_datapoints_in_package); i++)
                        {
                            try
                            {
                                anc_packages[i][temperature_channel] = std::stod (payload[i]);
                            }
                            catch (...)
                            {
                                safe_logger (spdlog::level::warn, "invalid data in payload: {}",
                                    payload[i].c_str ());
                            }
                        }
                    }
                }
                if (type_tag == EDA)
                {
                    int eda_channel = board_descr["ancillary"]["eda_channels"][0];
                    for (int i = 0; i < (int)payload.size (); i++)
                    {
                        anc_packages[i][board_descr["ancillary"]["timestamp_channel"].get<int> ()] =
                            get_timestamp ();
                        anc_packages[i]
                                    [board_descr["ancillary"]["package_num_channel"].get<int> ()] =
                                        package_num;
                        try
                        {
                            anc_packages[i][eda_channel] = std::stod (payload[i]);
                        }
                        catch (...)
                        {
                            safe_logger (spdlog::level::warn, "invalid data in payload: {}",
                                payload[i].c_str ());
                        }
                        push_package (anc_packages[i], (int)BrainFlowPresets::ANCILLARY_PRESET);
                    }
                }
            }
            else
            {
                safe_logger (
                    spdlog::level::trace, "invalid header for package: {}", recv_package.c_str ());
            }
        }
    }
    for (int cur_package = 0; cur_package < max_datapoints_in_package; cur_package++)
    {
        delete[] default_packages[cur_package];
        delete[] aux_packages[cur_package];
        delete[] anc_packages[cur_package];
    }
}

std::string Emotibit::create_package (const std::string &type_tag, uint16_t package_number,
    const std::string &data, uint16_t data_length, uint8_t protocol_version,
    uint8_t data_reliability)
{
    std::string header = create_header (
        type_tag, 0, package_number, data_length, protocol_version, data_reliability);
    if (data_length == 0)
    {
        return header + PACKET_DELIMITER_CSV;
    }
    else
    {
        return header + PAYLOAD_DELIMITER + data + PACKET_DELIMITER_CSV;
    }
}

std::string Emotibit::create_package (const std::string &type_tag, uint16_t package_number,
    std::vector<std::string> data, uint8_t protocol_version, uint8_t data_reliability)
{
    std::string package = create_header (
        type_tag, 0, package_number, (uint16_t)data.size (), protocol_version, data_reliability);
    for (std::string s : data)
    {
        package += PAYLOAD_DELIMITER + s;
    }
    package += PACKET_DELIMITER_CSV;
    return package;
}

std::string Emotibit::create_header (const std::string &type_tag, uint32_t timestamp,
    uint16_t package_number, uint16_t data_length, uint8_t protocol_version,
    uint8_t data_reliability)
{
    std::string header = "";
    header += std::to_string (timestamp);
    header += PAYLOAD_DELIMITER;
    header += std::to_string (package_number);
    header += PAYLOAD_DELIMITER;
    header += std::to_string (data_length);
    header += PAYLOAD_DELIMITER;
    header += type_tag;
    header += PAYLOAD_DELIMITER;
    header += std::to_string ((int)protocol_version);
    header += PAYLOAD_DELIMITER;
    header += std::to_string ((int)data_reliability);
    return header;
}

std::vector<std::string> Emotibit::split_string (const std::string &package, char delim)
{
    std::vector<std::string> result;
    size_t start;
    size_t end = 0;
    while ((start = package.find_first_not_of (delim, end)) != std::string::npos)
    {
        end = package.find (delim, start);
        std::string cur_str = package.substr (start, end - start);
        result.push_back (cur_str);
    }
    return result;
}

bool Emotibit::get_header (
    const std::string &package_string, int *package_num, int *data_len, std::string &type_tag)
{
    std::string serial_number = "";
    return get_header (package_string, package_num, data_len, type_tag, serial_number);
}

bool Emotibit::get_header (const std::string &package_string, int *package_num, int *data_len,
    std::string &type_tag, std::string &serial_number)
{
    std::vector<std::string> package = split_string (package_string, PAYLOAD_DELIMITER);
    if (package.size () >= HEADER_LENGTH)
    {
        try
        {
            if (package.at (1) != "")
            {
                *package_num = stoi (package.at (1));
            }
            else
            {
                return false;
            }
            if (package.at (2) != "")
            {
                *data_len = stoi (package.at (2));
            }
            else
            {
                return false;
            }
            if (package.at (3) != "")
            {
                type_tag = package.at (3);
                if (type_tag == HELLO_HOST)
                {
                    if (package.size () > 9)
                    {
                        serial_number = package.at (9);
                    }
                }
            }
            else
            {
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    else
    {
        safe_logger (
            spdlog::level::warn, "package size: {} is smaller than expected", package.size ());
        return false;
    }

    if (package.size () < (size_t)HEADER_LENGTH + *data_len)
    {
        safe_logger (spdlog::level::warn, "small package: {}", package_string.c_str ());
        return false;
    }
    else
    {
        return true;
    }
}

std::vector<std::string> Emotibit::get_payload (const std::string &package_string, int data_len)
{
    std::vector<std::string> res;
    std::vector<std::string> package = split_string (package_string, PAYLOAD_DELIMITER);
    if (package.size () >= (size_t)HEADER_LENGTH + (size_t)data_len)
    {
        res.insert (res.begin (), package.begin () + HEADER_LENGTH,
            package.begin () + (size_t)HEADER_LENGTH + (size_t)data_len);
    }
    return res;
}

int Emotibit::create_adv_connection ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::vector<std::string> broadcast_addresses;
    if (!params.ip_address.empty ())
    {
        broadcast_addresses.push_back (params.ip_address);
    }
    else
    {
        safe_logger (spdlog::level::warn,
            "no ip_address provided, trying to discover network, it may take longer");
        broadcast_addresses = get_broadcast_addresses ();
    }

    if (broadcast_addresses.empty ())
    {
        safe_logger (spdlog::level::err, "no broadcast addresses found");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    for (std::string broadcast_address : broadcast_addresses)
    {
        BroadCastServer *advertise_socket =
            new BroadCastServer (broadcast_address.c_str (), WIFI_ADVERTISING_PORT);
        res = (int)BrainFlowExitCodes::STATUS_OK;
        safe_logger (
            spdlog::level::info, "trying broadcast address: {}", broadcast_address.c_str ());
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            int init_res = advertise_socket->init ();
            if (init_res != (int)BroadCastServerReturnCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::err, "failed to init broadcast server socket: {}", res);
                res = (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
        }
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            std::string package = create_package (HELLO_EMOTIBIT, 0, "", 0);
            safe_logger (spdlog::level::info, "sending package: {}", package.c_str ());
            int bytes_send = advertise_socket->send (package.c_str (), (int)package.size ());
            if (bytes_send != (int)package.size ())
            {
                safe_logger (
                    spdlog::level::err, "failed to send adv package, res is {}", bytes_send);
                res = (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
        }
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            constexpr int max_size = 32768;
            constexpr int max_ip_addr_size = 100;
            char recv_data[max_size];
            char emotibit_ip[max_ip_addr_size];
            bool found = false;
            double start_time = get_timestamp ();
            for (int i = 0; (i < 100) && (!found); i++)
            {
                int bytes_recv =
                    advertise_socket->recv (recv_data, max_size, emotibit_ip, max_ip_addr_size);
                if (bytes_recv > 0)
                {
                    std::vector<std::string> splitted_packages =
                        split_string (std::string (recv_data, bytes_recv), PACKET_DELIMITER_CSV);
                    for (std::string recv_package : splitted_packages)
                    {
                        safe_logger (spdlog::level::trace, "package is {}", recv_package.c_str ());
                        int package_num = 0;
                        int data_len = 0;
                        std::string type_tag = "";
                        std::string serial_number = "";
                        if (get_header (
                                recv_package, &package_num, &data_len, type_tag, serial_number))
                        {
                            safe_logger (spdlog::level::info, "received {} package", type_tag);
                            if (type_tag == HELLO_HOST)
                            {
                                safe_logger (
                                    spdlog::level::info, "Found emotibit: {}", serial_number);
                                if (params.serial_number.empty () ||
                                    (params.serial_number == serial_number))
                                {
                                    found = true;
                                    ip_address = emotibit_ip;
                                }
                            }
                        }
                        else
                        {
                            safe_logger (spdlog::level::trace, "invalid header, package is: {}",
                                recv_package.c_str ());
                        }
                    }
                }
                if (get_timestamp () - start_time > params.timeout)
                {
                    break;
                }
            }
            if (!found)
            {
                safe_logger (spdlog::level::err, "no emotibit found");
                res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
            }
        }
        if (advertise_socket != NULL)
        {
            delete advertise_socket;
            advertise_socket = NULL;
        }
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            break;
        }
    }
    // we dont need broadcast anymore, replace it with normal socket
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // ugly but makes it safer
#ifdef _WIN32
        Sleep (1000);
#else
        usleep (1000000);
#endif
        adv_socket = new SocketClientUDP (ip_address.c_str (), WIFI_ADVERTISING_PORT);
        if (adv_socket->connect () != ((int)SocketClientUDPReturnCodes::STATUS_OK))
        {
            safe_logger (spdlog::level::err, "Failed to bind adv_socket");
            res = (int)BrainFlowExitCodes::SET_PORT_ERROR;
            adv_socket->close ();
            delete adv_socket;
            adv_socket = NULL;
        }
        else
        {
            safe_logger (spdlog::level::debug, "adv_connection established, ip: {}, port: {}",
                ip_address, WIFI_ADVERTISING_PORT);
        }
    }
    return res;
}

int Emotibit::create_data_connection ()
{
    int res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    for (int i = 2; i < 40; i += 2)
    {
        data_port = WIFI_ADVERTISING_PORT + i;
        data_socket = new SocketClientUDP (ip_address.c_str (), data_port);
        if (data_socket->bind () == ((int)SocketClientUDPReturnCodes::STATUS_OK))
        {
            safe_logger (spdlog::level::info, "use port {} for data", data_port);
            res = (int)BrainFlowExitCodes::STATUS_OK;
            break;
        }
        else
        {
            safe_logger (spdlog::level::warn, "failed to bind to {}", data_port);
        }
        data_socket->close ();
        delete data_socket;
        data_socket = NULL;
    }
    return res;
}

int Emotibit::create_control_connection ()
{
    char local_ip[80];
    int local_ip_res =
        SocketClientUDP::get_local_ip_addr (ip_address.c_str (), WIFI_ADVERTISING_PORT, local_ip);
    if (local_ip_res != (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to get local ip addr: {}", local_ip_res);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    safe_logger (spdlog::level::info, "local ip addr is {}", local_ip);

    int res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    for (int i = 1; i < 39; i += 2)
    {
        control_port = WIFI_ADVERTISING_PORT + i;
        control_socket = new SocketServerTCP (local_ip, control_port, true);
        if (control_socket->bind () == ((int)SocketServerTCPReturnCodes::STATUS_OK))
        {
            safe_logger (spdlog::level::info, "use port {} for control", control_port);
            res = (int)BrainFlowExitCodes::STATUS_OK;
            break;
        }
        else
        {
            safe_logger (spdlog::level::warn, "failed to connect to {}", control_port);
        }
        control_socket->close ();
        delete control_socket;
        control_socket = NULL;
    }
    return res;
}

int Emotibit::send_connect_msg ()
{
    // should never happen
    if ((control_port < 0) || (data_port < 0))
    {
        safe_logger (spdlog::level::info, "ports for data or control are not set");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    std::vector<std::string> payload;
    payload.push_back (CONTROL_PORT);
    payload.push_back (std::to_string (control_port));
    payload.push_back (DATA_PORT);
    payload.push_back (std::to_string (data_port));
    std::string package = create_package (EMOTIBIT_CONNECT, 0, payload);
    safe_logger (spdlog::level::info, "sending connect package: {}", package.c_str ());

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int bytes_send = adv_socket->send (package.c_str (), (int)package.size ());
    if (bytes_send != (int)package.size ())
    {
        safe_logger (spdlog::level::err, "failed to send connect package, res is {}", bytes_send);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return res;
}

int Emotibit::send_control_msg (const char *msg)
{
    // should never happen
    if ((control_port < 0) || (data_port < 0))
    {
        safe_logger (spdlog::level::info, "ports for data or control are not set");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    std::string package = create_package (msg, 0, "", 0);

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int bytes_send = control_socket->send (package.c_str (), (int)package.size ());
    if (bytes_send != (int)package.size ())
    {
        safe_logger (spdlog::level::err, "failed to send control msg package: {}, res is {}", msg,
            bytes_send);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        safe_logger (spdlog::level::info, "Message: {} sent", msg);
    }
    return res;
}

int Emotibit::wait_for_connection ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int accept_res = control_socket->accept ();
    if (accept_res != (int)SocketServerTCPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "error in accept");
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        int max_attempts = 20;
        for (int i = 0; i < max_attempts; i++)
        {
            safe_logger (spdlog::level::trace, "waiting for accept {}/{}", i, max_attempts);
            if (control_socket->client_connected)
            {
                safe_logger (spdlog::level::trace, "emotibit connected");
                break;
            }
            else
            {
#ifdef _WIN32
                Sleep (300);
#else
                usleep (300000);
#endif
            }
        }
        if (!control_socket->client_connected)
        {
            safe_logger (spdlog::level::trace, "failed to establish connection");
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }
    return res;
}

void Emotibit::ping_thread ()
{
    int sleep_time = 1;
    int package_num = 1;
    while (initialized)
    {
#ifdef _WIN32
        Sleep (sleep_time * 1000);
#else
        usleep (sleep_time * 1000000);
#endif
        std::vector<std::string> payload;
        payload.push_back (DATA_PORT);
        payload.push_back (std::to_string (data_port));
        std::string package = create_package (PING, package_num++, payload);
        // safe_logger (spdlog::level::trace, "sending package: {}", package.c_str ());
        int bytes_send = adv_socket->send (package.c_str (), (int)package.size ());
        if (bytes_send != (int)package.size ())
        {
            safe_logger (spdlog::level::err, "failed to send adv package, res is {}", bytes_send);
        }
    }
}
