#include "ant_neuro_edx.h"

#ifdef BUILD_ANT_EDX

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <regex>
#include <set>
#include <sstream>
#include <unordered_map>

#include "json.hpp"
#include "timestamp.h"

using json = nlohmann::json;

namespace
{
bool is_ant_master_board (int board_id)
{
    return ((board_id >= (int)BoardIds::ANT_NEURO_EE_410_BOARD &&
               board_id <= (int)BoardIds::ANT_NEURO_EE_225_BOARD) ||
        (board_id == (int)BoardIds::ANT_NEURO_EE_511_BOARD));
}

int explicit_edx_to_master_board (int board_id)
{
    switch ((BoardIds)board_id)
    {
        case BoardIds::ANT_NEURO_EE_410_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_410_BOARD;
        case BoardIds::ANT_NEURO_EE_411_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_411_BOARD;
        case BoardIds::ANT_NEURO_EE_430_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_430_BOARD;
        case BoardIds::ANT_NEURO_EE_211_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_211_BOARD;
        case BoardIds::ANT_NEURO_EE_212_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_212_BOARD;
        case BoardIds::ANT_NEURO_EE_213_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_213_BOARD;
        case BoardIds::ANT_NEURO_EE_214_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_214_BOARD;
        case BoardIds::ANT_NEURO_EE_215_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_215_BOARD;
        case BoardIds::ANT_NEURO_EE_221_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_221_BOARD;
        case BoardIds::ANT_NEURO_EE_222_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_222_BOARD;
        case BoardIds::ANT_NEURO_EE_223_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_223_BOARD;
        case BoardIds::ANT_NEURO_EE_224_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_224_BOARD;
        case BoardIds::ANT_NEURO_EE_225_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_225_BOARD;
        case BoardIds::ANT_NEURO_EE_511_EDX_BOARD:
            return (int)BoardIds::ANT_NEURO_EE_511_BOARD;
        default:
            return (int)BoardIds::NO_BOARD;
    }
}

std::string to_upper (std::string s)
{
    std::transform (s.begin (), s.end (), s.begin (),
        [] (unsigned char c) { return (char)std::toupper (c); });
    return s;
}

std::vector<std::string> expected_tokens (int master_board)
{
    switch ((BoardIds)master_board)
    {
        case BoardIds::ANT_NEURO_EE_511_BOARD:
            return {"EE-511", "EE-5XX"};
        case BoardIds::ANT_NEURO_EE_410_BOARD:
        case BoardIds::ANT_NEURO_EE_411_BOARD:
        case BoardIds::ANT_NEURO_EE_430_BOARD:
            return {"EE-4XX"};
        default:
            return {"EE-2XX"};
    }
}

std::set<std::string> extract_tokens (const std::string &value)
{
    std::set<std::string> result;
    std::regex re ("EE[\\-_ ]?([245][0-9X]{2})");
    std::string upper = to_upper (value);
    auto begin = std::sregex_iterator (upper.begin (), upper.end (), re);
    auto end = std::sregex_iterator ();
    for (auto it = begin; it != end; ++it)
    {
        std::string suffix = (*it)[1].str ();
        std::string token = "EE-" + suffix;
        result.insert (token);
        if (suffix.size () == 3 && std::isdigit ((unsigned char)suffix[0]))
        {
            result.insert (std::string ("EE-") + suffix[0] + "XX");
        }
    }
    return result;
}

bool has_match (const std::set<std::string> &tokens, const std::vector<std::string> &need)
{
    for (const auto &token : need)
    {
        if (tokens.find (token) != tokens.end ())
        {
            return true;
        }
    }
    return false;
}

double ts_to_unix (const google::protobuf::Timestamp &ts)
{
    return (double)ts.seconds () + ((double)ts.nanos () / 1000000000.0);
}

int map_status (const grpc::Status &status)
{
    if (status.ok ())
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    Board::board_logger->error ("gRPC error: code={} message='{}' details='{}'",
        (int)status.error_code (), status.error_message (), status.error_details ());
    if (status.error_code () == grpc::StatusCode::DEADLINE_EXCEEDED)
    {
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
    if (status.error_code () == grpc::StatusCode::INVALID_ARGUMENT)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
}

std::vector<int> try_get_vec (const json &obj, const char *key)
{
    try
    {
        return obj[key].get<std::vector<int>> ();
    }
    catch (...)
    {
        return {};
    }
}

size_t expected_active_channel_count (const json &board_default)
{
    std::set<int> channels;
    const char *keys[] = {"eeg_channels", "emg_channels", "ecg_channels", "eog_channels"};
    for (const char *key : keys)
    {
        for (int channel : try_get_vec (board_default, key))
        {
            channels.insert (channel);
        }
    }
    return channels.size ();
}
} // namespace

AntNeuroEdxBoard::AntNeuroEdxBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
    keep_alive = false;
    initialized = false;
    is_streaming = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    amplifier_handle = -1;
    requested_master_board = explicit_edx_to_master_board (board_id);
    package_num = 0;
    impedance_mode = false;
    sampling_rate = -1;
    reference_range = -1.0;
    bipolar_range = -1.0;
    trigger_channel_index = -1;
    missing_start_frame_count = 0;
    fallback_timestamp_count = 0;
    non_monotonic_timestamp_count = 0;
    large_gap_count = 0;
    last_emitted_timestamp = -1.0;
    impedance_sample_count = 0;
    mode_request.store (EdxModeRequest::None);
    mode_result.store ((int)BrainFlowExitCodes::STATUS_OK);
#ifdef BUILD_ANT_EDX
    streaming_context = nullptr;
#endif
}

AntNeuroEdxBoard::~AntNeuroEdxBoard ()
{
    skip_logs = true;
    release_session ();
}

int AntNeuroEdxBoard::validate_master_board ()
{
    if (requested_master_board == (int)BoardIds::NO_BOARD)
    {
        safe_logger (spdlog::level::err,
            "failed to map explicit EDX board {} to ANT master board", board_id);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if ((params.master_board != (int)BoardIds::NO_BOARD) &&
        (params.master_board != requested_master_board))
    {
        safe_logger (spdlog::level::err,
            "explicit EDX board {} conflicts with master_board {} (expected {})",
            board_id, params.master_board, requested_master_board);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (!is_ant_master_board (requested_master_board))
    {
        safe_logger (spdlog::level::err, "invalid master_board for EDX: {}", requested_master_board);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::ensure_connected ()
{
    if (!params.other_info.empty () && params.ip_address.empty () && (params.ip_port <= 0))
    {
        safe_logger (spdlog::level::err,
            "EDX endpoint in other_info is no longer supported, use ip_address/ip_port");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (params.ip_address.empty ())
    {
        safe_logger (spdlog::level::err, "EDX requires ip_address");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if ((params.ip_port <= 0) || (params.ip_port > 65535))
    {
        safe_logger (spdlog::level::err, "EDX requires valid ip_port (1..65535), got {}", params.ip_port);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (!params.other_info.empty ())
    {
        safe_logger (spdlog::level::warn,
            "EDX endpoint in other_info is no longer supported, use ip_address/ip_port");
    }

    if ((params.ip_address.find ("://") != std::string::npos) ||
        (params.ip_address.find ("/") != std::string::npos))
    {
        safe_logger (spdlog::level::err,
            "EDX requires host-only ip_address, got URI-like value '{}'",
            params.ip_address);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    endpoint = "dns:///" + params.ip_address + ":" + std::to_string (params.ip_port);

    grpc_channel = grpc::CreateChannel (endpoint, grpc::InsecureChannelCredentials ());
    stub = EdigRPC::gen::EdigRPC::NewStub (grpc_channel);
    return stub ? (int)BrainFlowExitCodes::STATUS_OK : (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
}

int AntNeuroEdxBoard::connect_and_create_device ()
{
    EdigRPC::gen::DeviceManager_GetDevicesRequest req;
    EdigRPC::gen::DeviceManager_GetDevicesResponse resp;
    grpc::ClientContext ctx;
    ctx.set_deadline (
        std::chrono::system_clock::now () + std::chrono::seconds (std::max (1, params.timeout)));
    grpc::Status status = stub->DeviceManager_GetDevices (&ctx, req, &resp);
    if (!status.ok ())
    {
        return map_status (status);
    }

    std::vector<std::string> need = expected_tokens (requested_master_board);
    std::vector<EdigRPC::gen::DeviceInfo> matched;
    std::vector<EdigRPC::gen::DeviceInfo> serial_matched;

    for (const auto &info : resp.deviceinfolist ())
    {
        std::set<std::string> tokens = extract_tokens (info.key () + " " + info.serial ());
        if (!has_match (tokens, need))
        {
            continue;
        }
        matched.push_back (info);
        if (!params.serial_number.empty ())
        {
            std::string serial_u = to_upper (params.serial_number);
            if (to_upper (info.key ()).find (serial_u) != std::string::npos ||
                to_upper (info.serial ()).find (serial_u) != std::string::npos)
            {
                serial_matched.push_back (info);
            }
        }
    }

    if (matched.empty () || (!params.serial_number.empty () && serial_matched.empty ()))
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    EdigRPC::gen::DeviceInfo selected =
        (!params.serial_number.empty ()) ? serial_matched.front () : matched.front ();
    selected_device_key = selected.key ();
    selected_device_serial = selected.serial ();
    std::set<std::string> tokens = extract_tokens (selected_device_key);
    if (!tokens.empty ())
    {
        selected_model = *tokens.begin ();
    }

    EdigRPC::gen::Controller_CreateDeviceRequest create_req;
    *create_req.add_deviceinfolist () = selected;
    EdigRPC::gen::Controller_CreateDeviceResponse create_resp;
    grpc::ClientContext create_ctx;
    create_ctx.set_deadline (
        std::chrono::system_clock::now () + std::chrono::seconds (std::max (1, params.timeout)));
    status = stub->Controller_CreateDevice (&create_ctx, create_req, &create_resp);
    if (!status.ok ())
    {
        return map_status (status);
    }

    amplifier_handle = create_resp.amplifierhandle ();
    return (amplifier_handle < 0) ? (int)BrainFlowExitCodes::GENERAL_ERROR :
                                    (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::load_capabilities ()
{
    EdigRPC::gen::Amplifier_GetChannelsAvailableRequest channels_req;
    channels_req.set_amplifierhandle (amplifier_handle);
    EdigRPC::gen::Amplifier_GetChannelsAvailableResponse channels_resp;
    grpc::ClientContext channels_ctx;
    channels_ctx.set_deadline (std::chrono::system_clock::now () +
        std::chrono::seconds (std::max (1, params.timeout)));
    grpc::Status status = stub->Amplifier_GetChannelsAvailable (&channels_ctx, channels_req, &channels_resp);
    if (!status.ok ())
    {
        return map_status (status);
    }

    channel_meta.clear ();
    active_channel_indices.clear ();
    trigger_channel_index = -1;
    for (const auto &channel : channels_resp.channellist ())
    {
        EdxChannelMeta meta;
        meta.index = channel.channelindex ();
        meta.polarity = channel.channelpolarity ();
        meta.name = channel.name ();
        channel_meta.push_back (meta);
        if (meta.polarity == EdigRPC::gen::ChannelPolarity::Referential ||
            meta.polarity == EdigRPC::gen::ChannelPolarity::Bipolar)
        {
            active_channel_indices.push_back (meta.index);
        }
        if (to_upper (meta.name).find ("TRIGGER") != std::string::npos ||
            meta.polarity == EdigRPC::gen::ChannelPolarity::Receiver ||
            meta.polarity == EdigRPC::gen::ChannelPolarity::Transmitter)
        {
            trigger_channel_index = meta.index;
        }
    }

    EdigRPC::gen::Amplifier_GetSamplingRatesAvailableRequest rates_req;
    rates_req.set_amplifierhandle (amplifier_handle);
    EdigRPC::gen::Amplifier_GetSamplingRatesAvailableResponse rates_resp;
    grpc::ClientContext rates_ctx;
    rates_ctx.set_deadline (std::chrono::system_clock::now () +
        std::chrono::seconds (std::max (1, params.timeout)));
    status = stub->Amplifier_GetSamplingRatesAvailable (&rates_ctx, rates_req, &rates_resp);
    if (!status.ok ())
    {
        return map_status (status);
    }
    sampling_rates_available.clear ();
    for (double rate : rates_resp.ratelist ())
    {
        sampling_rates_available.push_back ((int)std::lround (rate));
    }
    if (!sampling_rates_available.empty ())
    {
        sampling_rate = *std::max_element (
            sampling_rates_available.begin (), sampling_rates_available.end ());
    }

    EdigRPC::gen::Amplifier_GetRangesAvailableRequest ranges_req;
    ranges_req.set_amplifierhandle (amplifier_handle);
    EdigRPC::gen::Amplifier_GetRangesAvailableResponse ranges_resp;
    grpc::ClientContext ranges_ctx;
    ranges_ctx.set_deadline (std::chrono::system_clock::now () +
        std::chrono::seconds (std::max (1, params.timeout)));
    status = stub->Amplifier_GetRangesAvailable (&ranges_ctx, ranges_req, &ranges_resp);
    if (!status.ok ())
    {
        return map_status (status);
    }
    reference_ranges_available.clear ();
    bipolar_ranges_available.clear ();
    for (const auto &entry : ranges_resp.rangemap ())
    {
        if (entry.first == (int)EdigRPC::gen::ChannelPolarity::Referential)
        {
            reference_ranges_available.assign (entry.second.values ().begin (), entry.second.values ().end ());
        }
        else if (entry.first == (int)EdigRPC::gen::ChannelPolarity::Bipolar)
        {
            bipolar_ranges_available.assign (entry.second.values ().begin (), entry.second.values ().end ());
        }
    }

    EdigRPC::gen::Amplifier_GetModesAvailableRequest modes_req;
    modes_req.set_amplifierhandle (amplifier_handle);
    EdigRPC::gen::Amplifier_GetModesAvailableResponse modes_resp;
    grpc::ClientContext modes_ctx;
    modes_ctx.set_deadline (std::chrono::system_clock::now () +
        std::chrono::seconds (std::max (1, params.timeout)));
    status = stub->Amplifier_GetModesAvailable (&modes_ctx, modes_req, &modes_resp);
    if (!status.ok ())
    {
        return map_status (status);
    }
    modes_available.clear ();
    for (auto mode : modes_resp.modelist ())
    {
        modes_available.push_back ((EdigRPC::gen::AmplifierMode)mode);
    }

    if (reference_range <= 0.0 && !reference_ranges_available.empty ())
    {
        reference_range = reference_ranges_available.front ();
    }
    if (bipolar_range <= 0.0 && !bipolar_ranges_available.empty ())
    {
        bipolar_range = bipolar_ranges_available.front ();
    }
    if (selected_model == "EE-511" || to_upper (selected_device_serial).find ("EE511") != std::string::npos)
    {
        reference_range = 1.0;
        bipolar_range = 2.5;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::prepare_session ()
{
    if (initialized)
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    int res = validate_master_board ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    res = ensure_connected ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    try
    {
        int descr_board_id = board_id;
        board_descr =
            boards_struct.brainflow_boards_json["boards"][std::to_string (descr_board_id)];
        sampling_rate = board_descr["default"]["sampling_rate"];
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    res = connect_and_create_device ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    res = load_capabilities ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        release_session ();
        return res;
    }
    size_t expected_channels = expected_active_channel_count (board_descr["default"]);
    if (expected_channels > 0 && expected_channels != active_channel_indices.size ())
    {
        safe_logger (spdlog::level::err,
            "EDX board {} expected {} active EXG channels from descriptor but device reports {}",
            board_id, expected_channels, active_channel_indices.size ());
        release_session ();
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    int expected_rate = board_descr["default"]["sampling_rate"];
    if (!sampling_rates_available.empty () &&
        std::find (sampling_rates_available.begin (), sampling_rates_available.end (), expected_rate) ==
            sampling_rates_available.end ())
    {
        std::ostringstream rates_stream;
        for (size_t i = 0; i < sampling_rates_available.size (); i++)
        {
            if (i > 0)
            {
                rates_stream << ", ";
            }
            rates_stream << sampling_rates_available[i];
        }
        safe_logger (spdlog::level::err,
            "EDX board {} expected sampling rate {} but device supports [{}]",
            board_id, expected_rate, rates_stream.str ());
        release_session ();
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    // Device-discovered capabilities refine validation and configuration.
    // Explicit EDX board ids remain self-describing.

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::configure_stream_params (void *request_ptr)
{
    EdigRPC::gen::Amplifier_SetModeRequest *request =
        static_cast<EdigRPC::gen::Amplifier_SetModeRequest *> (request_ptr);
    auto *stream_params = request->mutable_streamparams ();
    stream_params->set_samplingrate (sampling_rate);
    stream_params->set_datareadypercentage (5);
    stream_params->set_buffersize (1024);
    stream_params->clear_ranges ();
    for (int channel_index : active_channel_indices)
    {
        stream_params->add_activechannels (channel_index);
    }
    if (reference_range > 0.0)
    {
        (*stream_params->mutable_ranges ())[(int)EdigRPC::gen::ChannelPolarity::Referential] =
            reference_range;
    }
    if (bipolar_range > 0.0)
    {
        (*stream_params->mutable_ranges ())[(int)EdigRPC::gen::ChannelPolarity::Bipolar] =
            bipolar_range;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::set_mode ()
{
    EdigRPC::gen::Amplifier_SetModeRequest request;
    request.set_amplifierhandle (amplifier_handle);
    request.set_mode (
        impedance_mode ? EdigRPC::gen::AmplifierMode::AmplifierMode_Impedance :
                         EdigRPC::gen::AmplifierMode::AmplifierMode_Eeg);
    configure_stream_params (&request);

    EdigRPC::gen::Amplifier_SetModeResponse response;
    grpc::ClientContext ctx;
    ctx.set_deadline (
        std::chrono::system_clock::now () + std::chrono::seconds (std::max (1, params.timeout)));
    grpc::Status status = stub->Amplifier_SetMode (&ctx, request, &response);
    return map_status (status);
}

int AntNeuroEdxBoard::set_idle_mode ()
{
    if (!stub || amplifier_handle < 0)
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    EdigRPC::gen::Amplifier_SetModeRequest request;
    request.set_amplifierhandle (amplifier_handle);
    request.set_mode (EdigRPC::gen::AmplifierMode::AmplifierMode_Idle);
    // Allocate an empty StreamParams so the server doesn't dereference null,
    // but don't populate channels/rates — idle doesn't need them.
    request.mutable_streamparams ();
    EdigRPC::gen::Amplifier_SetModeResponse response;

    grpc::ClientContext ctx;
    ctx.set_deadline (
        std::chrono::system_clock::now () + std::chrono::seconds (std::max (1, params.timeout)));
    grpc::Status status = stub->Amplifier_SetMode (&ctx, request, &response);
    return map_status (status);
}

int AntNeuroEdxBoard::apply_mode_change ()
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    bool was_streaming = is_streaming || keep_alive;
    if (was_streaming)
    {
        safe_logger (spdlog::level::info, "EDX apply_mode_change: stopping current stream (impedance_mode={})",
            impedance_mode ? 1 : 0);
        keep_alive = false;
        is_streaming = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }
    }

    safe_logger (spdlog::level::info, "EDX apply_mode_change: calling set_mode (impedance_mode={})",
        impedance_mode ? 1 : 0);
    int res = set_mode ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "EDX apply_mode_change: set_mode failed with {}", res);
        return res;
    }

    if (!was_streaming)
    {
        safe_logger (spdlog::level::info, "EDX apply_mode_change: was not streaming, done");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    last_emitted_timestamp = -1.0;
    non_monotonic_timestamp_count = 0;
    large_gap_count = 0;
    fallback_timestamp_count = 0;
    missing_start_frame_count = 0;
    impedance_sample_count = 0;
    keep_alive = true;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    streaming_thread = std::thread ([this] { read_thread (); });

    // Impedance mode needs longer to produce first frame (amplifier settling)
    int wait_secs = std::max (1, params.timeout);
    if (impedance_mode && wait_secs < 30)
    {
        wait_secs = 30;
    }
    safe_logger (spdlog::level::info, "EDX apply_mode_change: waiting up to {}s for first frame", wait_secs);
    std::unique_lock<std::mutex> lk (wait_mutex);
    if (wait_cv.wait_for (
            lk, std::chrono::seconds (wait_secs),
            [this] { return state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        if (state == (int)BrainFlowExitCodes::STATUS_OK)
        {
            is_streaming = true;
            safe_logger (spdlog::level::info, "EDX apply_mode_change: streaming started");
        }
        return state;
    }

    safe_logger (spdlog::level::err, "EDX apply_mode_change: timed out after {}s waiting for first frame", wait_secs);
    keep_alive = false;
    if (streaming_thread.joinable ())
    {
        streaming_thread.join ();
    }
    return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

void AntNeuroEdxBoard::read_thread ()
{
    int sleep_time_ms = impedance_mode ? 100 : 5;
    int wait_attempts = 0;
    int wait_secs = std::max (1, params.timeout);
    if (impedance_mode && wait_secs < 30)
    {
        wait_secs = 30;
    }
    int max_wait_attempts = wait_secs * 1000 / sleep_time_ms;

    while (keep_alive)
    {
        // Process any pending mode-change request between frames.
        // This is the ONLY safe place to call Amplifier_SetMode — the EDX
        // gRPC server corrupts its state if SetMode races with GetFrame.
        process_mode_request ();

        int res = process_frames ();
        if (res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            if (state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                {
                    std::lock_guard<std::mutex> lk (wait_mutex);
                    state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                wait_cv.notify_one ();
            }
            wait_attempts = 0;
        }
        else
        {
            wait_attempts++;
            if (wait_attempts >= max_wait_attempts)
            {
                {
                    std::lock_guard<std::mutex> lk (wait_mutex);
                    state = res;
                }
                wait_cv.notify_one ();
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (sleep_time_ms));
        }
    }

    // Handle any pending request on exit
    process_mode_request ();
}

void AntNeuroEdxBoard::process_mode_request ()
{
    EdxModeRequest req = mode_request.load ();
    if (req == EdxModeRequest::None)
    {
        return;
    }

    int res;
    switch (req)
    {
        case EdxModeRequest::Idle:
            res = set_idle_mode ();
            break;
        case EdxModeRequest::Eeg:
            impedance_mode = false;
            res = set_mode ();
            break;
        case EdxModeRequest::Impedance:
            impedance_mode = true;
            res = set_mode ();
            break;
        default:
            res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            break;
    }

    {
        std::lock_guard<std::mutex> lk (mode_mutex);
        mode_result.store (res);
        mode_request.store (EdxModeRequest::None);
    }
    mode_cv.notify_one ();
}

int AntNeuroEdxBoard::process_frames ()
{
    EdigRPC::gen::Amplifier_GetFrameRequest request;
    request.set_amplifierhandle (amplifier_handle);
    EdigRPC::gen::Amplifier_GetFrameResponse response;

    grpc::ClientContext ctx;
    // Impedance frames arrive less frequently; use longer deadline to avoid
    // premature DEADLINE_EXCEEDED that masks valid settling-time delays.
    int deadline_ms = impedance_mode ? 3000 : 500;
    ctx.set_deadline (std::chrono::system_clock::now () + std::chrono::milliseconds (deadline_ms));
    streaming_context = &ctx;
    grpc::Status status = stub->Amplifier_GetFrame (&ctx, request, &response);
    streaming_context = nullptr;
    if (!status.ok ())
    {
        return map_status (status);
    }
    if (response.framelist ().empty ())
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    int num_rows = board_descr["default"]["num_rows"];
    int package_num_channel = board_descr["default"]["package_num_channel"];
    int timestamp_channel = board_descr["default"]["timestamp_channel"];
    int marker_channel = board_descr["default"]["marker_channel"];
    std::vector<int> eeg_channels = try_get_vec (board_descr["default"], "eeg_channels");
    std::vector<int> emg_channels = try_get_vec (board_descr["default"], "emg_channels");
    std::vector<int> resistance_channels = try_get_vec (board_descr["default"], "resistance_channels");
    std::vector<int> ref_resistance_channels =
        try_get_vec (board_descr["default"], "ref_resistance_channels");
    std::vector<int> gnd_resistance_channels =
        try_get_vec (board_descr["default"], "gnd_resistance_channels");
    std::vector<int> other_channels = try_get_vec (board_descr["default"], "other_channels");
    std::vector<double> package ((size_t)num_rows, 0.0);
    const double sample_dt = (sampling_rate > 0) ? (1.0 / (double)sampling_rate) : 0.0;
    const double large_gap_threshold = 1.0;

    for (const auto &frame : response.framelist ())
    {
        const bool has_start = frame.has_start ();
        const double frame_base_ts = has_start ? ts_to_unix (frame.start ()) : get_timestamp ();
        const int frame_marker_count = frame.timemarkers_size ();
        if (!has_start)
        {
            missing_start_frame_count++;
        }

        bool impedance_frame =
            impedance_mode || frame.frametype () == EdigRPC::gen::AmplifierFrameType::AmplifierFrameType_ImpedanceVoltages;
        if (impedance_frame)
        {
            std::fill (package.begin (), package.end (), 0.0);
            for (size_t i = 0;
                 i < frame.impedance ().channels ().size () && i < resistance_channels.size ();
                 i++)
            {
                package[(size_t)resistance_channels[i]] =
                    (double)frame.impedance ().channels ((int)i).value ();
            }
            for (size_t i = 0;
                 i < frame.impedance ().reference ().size () &&
                     i < ref_resistance_channels.size ();
                 i++)
            {
                package[(size_t)ref_resistance_channels[i]] =
                    (double)frame.impedance ().reference ((int)i).value ();
            }
            for (size_t i = 0;
                 i < frame.impedance ().ground ().size () &&
                     i < gnd_resistance_channels.size ();
                 i++)
            {
                package[(size_t)gnd_resistance_channels[i]] =
                    (double)frame.impedance ().ground ((int)i).value ();
            }
            package[(size_t)package_num_channel] = (double)package_num++;
            package[(size_t)timestamp_channel] = frame_base_ts;
            if (!has_start)
            {
                fallback_timestamp_count++;
            }
            if (last_emitted_timestamp > 0.0)
            {
                if (package[(size_t)timestamp_channel] < last_emitted_timestamp)
                {
                    non_monotonic_timestamp_count++;
                }
                if ((package[(size_t)timestamp_channel] - last_emitted_timestamp) > large_gap_threshold)
                {
                    large_gap_count++;
                }
            }
            last_emitted_timestamp = package[(size_t)timestamp_channel];
            impedance_sample_count++;
            push_package (package.data ());
            continue;
        }

        int cols = frame.matrix ().cols ();
        int rows = frame.matrix ().rows ();
        if (cols <= 0 || rows <= 0 || cols * rows != frame.matrix ().data_size ())
        {
            return (int)BrainFlowExitCodes::GENERAL_ERROR;
        }

        // Map each TimeMarker to its sample row via timestamp offset, then inject
        // via insert_marker so push_package picks it up from the queue.
        // Using long long avoids int overflow when TimeMarker.Start is unset (zero).
        std::unordered_map<int, double> marker_at_row;
        if (frame_marker_count > 0 && has_start && sample_dt > 0.0)
        {
            for (int mi = 0; mi < frame_marker_count; mi++)
            {
                const auto &tm = frame.timemarkers (mi);
                double marker_ts = ts_to_unix (tm.start ());
                double offset = marker_ts - frame_base_ts;
                long long target_row_ll = (long long)std::round (offset / sample_dt);
                int target_row = (int)std::max (0LL, std::min ((long long)(rows - 1), target_row_ll));
                marker_at_row[target_row] = (double)tm.timemarkercode ();
            }
        }
        else if (frame_marker_count > 0)
        {
            // No frame Start or zero sample_dt: place first marker at row 0.
            marker_at_row[0] = (double)frame.timemarkers (0).timemarkercode ();
        }

        for (int row = 0; row < rows; row++)
        {
            std::fill (package.begin (), package.end (), 0.0);
            int eeg_counter = 0;
            int emg_counter = 0;
            for (int col = 0; col < cols; col++)
            {
                double value = frame.matrix ().data (row * cols + col);
                int channel_index = (col < (int)active_channel_indices.size ()) ?
                        active_channel_indices[(size_t)col] :
                        col;

                EdigRPC::gen::ChannelPolarity polarity = EdigRPC::gen::ChannelPolarity::Referential;
                for (const auto &meta : channel_meta)
                {
                    if (meta.index == channel_index)
                    {
                        polarity = meta.polarity;
                        break;
                    }
                }

                if (polarity == EdigRPC::gen::ChannelPolarity::Referential &&
                    eeg_counter < (int)eeg_channels.size ())
                {
                    package[(size_t)eeg_channels[(size_t)eeg_counter++]] = value;
                }
                else if (polarity == EdigRPC::gen::ChannelPolarity::Bipolar &&
                    emg_counter < (int)emg_channels.size ())
                {
                    package[(size_t)emg_channels[(size_t)emg_counter++]] = value;
                }
                if (channel_index == trigger_channel_index && !other_channels.empty ())
                {
                    package[(size_t)other_channels[0]] = value;
                }
            }
            package[(size_t)package_num_channel] = (double)package_num++;
            package[(size_t)timestamp_channel] = frame_base_ts + (double)row * sample_dt;
            if (!has_start)
            {
                fallback_timestamp_count++;
            }
            if (last_emitted_timestamp > 0.0)
            {
                if (package[(size_t)timestamp_channel] < last_emitted_timestamp)
                {
                    non_monotonic_timestamp_count++;
                }
                if ((package[(size_t)timestamp_channel] - last_emitted_timestamp) > large_gap_threshold)
                {
                    large_gap_count++;
                }
            }
            last_emitted_timestamp = package[(size_t)timestamp_channel];
            auto it = marker_at_row.find (row);
            if (it != marker_at_row.end ())
            {
                insert_marker (it->second, (int)BrainFlowPresets::DEFAULT_PRESET);
            }
            push_package (package.data ());
        }
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (is_streaming)
    {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    res = set_mode ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    last_emitted_timestamp = -1.0;
    non_monotonic_timestamp_count = 0;
    large_gap_count = 0;
    fallback_timestamp_count = 0;
    missing_start_frame_count = 0;
    keep_alive = true;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    streaming_thread = std::thread ([this] { read_thread (); });

    std::unique_lock<std::mutex> lk (wait_mutex);
    auto sec = std::chrono::seconds (std::max (1, params.timeout));
    if (wait_cv.wait_for (
            lk, sec, [this] { return state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        if (state == (int)BrainFlowExitCodes::STATUS_OK)
        {
            is_streaming = true;
        }
        return state;
    }

    keep_alive = false;
    if (streaming_thread.joinable ())
    {
        streaming_thread.join ();
    }
    free_packages ();
    return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

// Request a mode change from the read_thread, which processes it between
// Amplifier_GetFrame calls where the gRPC server state is clean.
// If the thread is not running, calls the mode function directly.
// Waits up to timeout seconds for the thread to process the request.
int AntNeuroEdxBoard::request_mode_from_thread (EdxModeRequest req)
{
    if (!keep_alive)
    {
        // Thread not running — call directly (no race possible)
        switch (req)
        {
            case EdxModeRequest::Idle:
                return set_idle_mode ();
            case EdxModeRequest::Eeg:
                impedance_mode = false;
                return set_mode ();
            case EdxModeRequest::Impedance:
                impedance_mode = true;
                return set_mode ();
            default:
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    // Post the request for the read_thread
    mode_request.store (req);

    // Wait for the thread to process it
    std::unique_lock<std::mutex> lk (mode_mutex);
    auto deadline = std::chrono::seconds (std::max (2, params.timeout));
    if (mode_cv.wait_for (lk, deadline,
            [this] { return mode_request.load () == EdxModeRequest::None; }))
    {
        return mode_result.load ();
    }

    safe_logger (spdlog::level::warn, "EDX mode change request timed out");
    mode_request.store (EdxModeRequest::None);
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
}

int AntNeuroEdxBoard::stop_stream ()
{
    if (!is_streaming && !keep_alive)
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }

    // Request idle mode via the read_thread state machine.
    // The thread processes it between GetFrame calls where the gRPC server
    // state is clean.  If it fails (server already broken from sample loss),
    // cancel the in-flight GetFrame so the thread can exit promptly.
    int idle_res = request_mode_from_thread (EdxModeRequest::Idle);
    if (idle_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::warn,
            "EDX set_idle via thread failed ({}), cancelling in-flight GetFrame", idle_res);
        if (streaming_context != nullptr)
        {
            streaming_context->TryCancel ();
        }
    }

    keep_alive = false;
    is_streaming = false;
    if (streaming_thread.joinable ())
    {
        streaming_thread.join ();
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::release_session ()
{
    if (is_streaming || keep_alive)
    {
        stop_stream ();
    }

    // Try to set idle and dispose with the current handle.
    int old_handle = amplifier_handle;
    int idle_res = set_idle_mode ();
    bool disposed = false;

    if (idle_res == (int)BrainFlowExitCodes::STATUS_OK && stub && amplifier_handle >= 0)
    {
        // Handle is good — dispose normally
        EdigRPC::gen::Amplifier_DisposeRequest request;
        request.set_amplifierhandle (amplifier_handle);
        EdigRPC::gen::Amplifier_DisposeResponse response;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status s = stub->Amplifier_Dispose (&ctx, request, &response);
        disposed = s.ok ();
    }

    if (!disposed && stub && old_handle >= 0)
    {
        // set_idle failed (server state corrupted, e.g. after sample loss).
        // Try dispose directly on the old handle without idle — the server
        // may still accept dispose even when SetMode fails.
        safe_logger (spdlog::level::warn,
            "EDX set_idle failed ({}), trying dispose directly on handle {}", idle_res, old_handle);
        EdigRPC::gen::Amplifier_DisposeRequest request;
        request.set_amplifierhandle (old_handle);
        EdigRPC::gen::Amplifier_DisposeResponse response;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status s = stub->Amplifier_Dispose (&ctx, request, &response);
        safe_logger (spdlog::level::info,
            "EDX direct dispose ok={} msg={}", s.ok (), s.error_message ());
        disposed = s.ok ();
    }

    if (!disposed && old_handle >= 0)
    {
        // Both idle and dispose failed on the old connection.
        // Open a fresh gRPC channel and try dispose with the OLD handle number.
        // Do NOT call connect_and_create_device — the server still holds the
        // old handle and would reject "Amplifier in use".
        safe_logger (spdlog::level::warn,
            "EDX direct dispose failed, trying fresh gRPC connection with old handle {}", old_handle);
        stub.reset ();
        grpc_channel.reset ();

        int conn_res = ensure_connected ();
        if (conn_res == (int)BrainFlowExitCodes::STATUS_OK)
        {
            // Try dispose with old handle on fresh connection
            EdigRPC::gen::Amplifier_DisposeRequest request;
            request.set_amplifierhandle (old_handle);
            EdigRPC::gen::Amplifier_DisposeResponse response;
            grpc::ClientContext ctx;
            ctx.set_deadline (std::chrono::system_clock::now () +
                std::chrono::seconds (std::max (1, params.timeout)));
            grpc::Status s = stub->Amplifier_Dispose (&ctx, request, &response);
            safe_logger (spdlog::level::info,
                "EDX fresh-conn dispose ok={} msg={}", s.ok (), s.error_message ());
            disposed = s.ok ();
        }
        else
        {
            safe_logger (spdlog::level::warn, "EDX recovery: ensure_connected failed = {}", conn_res);
        }
    }

    free_packages ();
    initialized = false;
    amplifier_handle = -1;
    stub.reset ();
    grpc_channel.reset ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

bool AntNeuroEdxBoard::parse_bool_flag (const std::string &value, bool &flag)
{
    if (value == "0")
    {
        flag = false;
        return true;
    }
    if (value == "1")
    {
        flag = true;
        return true;
    }
    return false;
}

int AntNeuroEdxBoard::validate_sampling_rate (int value)
{
    if (sampling_rates_available.empty ())
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (std::find (sampling_rates_available.begin (), sampling_rates_available.end (), value) !=
               sampling_rates_available.end ()) ?
        (int)BrainFlowExitCodes::STATUS_OK :
        (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

int AntNeuroEdxBoard::validate_reference_range (double value)
{
    if (reference_ranges_available.empty ())
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (std::find (reference_ranges_available.begin (), reference_ranges_available.end (), value) !=
               reference_ranges_available.end ()) ?
        (int)BrainFlowExitCodes::STATUS_OK :
        (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

int AntNeuroEdxBoard::validate_bipolar_range (double value)
{
    if (bipolar_ranges_available.empty ())
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (std::find (bipolar_ranges_available.begin (), bipolar_ranges_available.end (), value) !=
               bipolar_ranges_available.end ()) ?
        (int)BrainFlowExitCodes::STATUS_OK :
        (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

int AntNeuroEdxBoard::parse_edx_command (const std::string &config, std::string &response)
{
    std::vector<std::string> parts;
    std::stringstream ss (config);
    std::string token;
    while (std::getline (ss, token, ':'))
    {
        parts.push_back (token);
    }
    if (parts.size () < 2 || parts[0] != "edx")
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    json out;
    out["op"] = parts[1];
    if (parts[1] == "get_capabilities")
    {
        out["sampling_rates"] = sampling_rates_available;
        out["active_channels"] = active_channel_indices;
        out["selected_model"] = selected_model;
        out["reference_ranges"] = reference_ranges_available;
        out["bipolar_ranges"] = bipolar_ranges_available;
        json channels = json::array ();
        for (const auto &meta : channel_meta)
        {
            json ch;
            ch["index"] = meta.index;
            ch["name"] = meta.name;
            ch["polarity"] = (int)meta.polarity;
            channels.push_back (ch);
        }
        out["channels"] = channels;
    }
    else if (parts[1] == "get_mode")
    {
        EdigRPC::gen::Amplifier_GetModeRequest request;
        request.set_amplifierhandle (amplifier_handle);
        EdigRPC::gen::Amplifier_GetModeResponse mode_response;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status status = stub->Amplifier_GetMode (&ctx, request, &mode_response);
        if (!status.ok ())
        {
            return map_status (status);
        }
        std::vector<int> modes;
        for (auto mode : mode_response.modelist ())
        {
            modes.push_back ((int)mode);
        }
        out["mode_list"] = modes;
    }
    else if (parts[1] == "get_power")
    {
        EdigRPC::gen::Amplifier_GetPowerRequest request;
        request.set_amplifierhandle (amplifier_handle);
        EdigRPC::gen::Amplifier_GetPowerResponse power_response;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status status = stub->Amplifier_GetPower (&ctx, request, &power_response);
        if (!status.ok ())
        {
            return map_status (status);
        }
        if (power_response.powerlist_size () > 0)
        {
            out["battery_level"] = power_response.powerlist (0).batterylevel ();
        }
    }
    else if (parts[1] == "trigger_config" && parts.size () >= 3)
    {
        // "edx:trigger_config:<ch>,<dutyCycle>,<pulseFreq>,<pulseCount>,<burstFreq>,<burstCount>"
        std::vector<double> vals;
        std::stringstream csv (parts[2]);
        std::string item;
        while (std::getline (csv, item, ','))
        {
            vals.push_back (std::stod (item));
        }
        if (vals.size () < 6)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        EdigRPC::gen::Amplifier_SetOutputTriggerChannelsRequest request;
        request.set_amplifierhandle (amplifier_handle);
        auto *info = request.add_infos ();
        info->set_channelindex ((int)vals[0]);
        info->set_channeltype (EdigRPC::gen::OutputChannelType_TriggerOutput);
        auto *params_map = info->mutable_parameters ();
        (*params_map)["dutyCycle"] = vals[1];
        (*params_map)["pulseFrequency"] = vals[2];
        (*params_map)["pulseCount"] = vals[3];
        (*params_map)["burstFrequency"] = vals[4];
        (*params_map)["burstCount"] = vals[5];
        EdigRPC::gen::Amplifier_SetOutputTriggerChannelsResponse resp;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status status = stub->Amplifier_SetOutputTriggerChannels (&ctx, request, &resp);
        if (!status.ok ())
        {
            return map_status (status);
        }
        out["channel"] = (int)vals[0];
    }
    else if (parts[1] == "trigger_start" && parts.size () >= 3)
    {
        EdigRPC::gen::Amplifier_StartOutputTriggerRequest request;
        request.set_amplifierhandle (amplifier_handle);
        std::stringstream csv (parts[2]);
        std::string item;
        while (std::getline (csv, item, ','))
        {
            request.add_channels (std::stoi (item));
        }
        EdigRPC::gen::Amplifier_StartOutputTriggerResponse resp;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status status = stub->Amplifier_StartOutputTrigger (&ctx, request, &resp);
        if (!status.ok ())
        {
            return map_status (status);
        }
    }
    else if (parts[1] == "trigger_stop" && parts.size () >= 3)
    {
        EdigRPC::gen::Amplifier_StopOutputTriggerRequest request;
        request.set_amplifierhandle (amplifier_handle);
        std::stringstream csv (parts[2]);
        std::string item;
        while (std::getline (csv, item, ','))
        {
            request.add_channels (std::stoi (item));
        }
        EdigRPC::gen::Amplifier_StopOutputTriggerResponse resp;
        grpc::ClientContext ctx;
        ctx.set_deadline (std::chrono::system_clock::now () +
            std::chrono::seconds (std::max (1, params.timeout)));
        grpc::Status status = stub->Amplifier_StopOutputTrigger (&ctx, request, &resp);
        if (!status.ok ())
        {
            return map_status (status);
        }
    }
    else
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    out["status"] = "ok";
    response = out.dump ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroEdxBoard::config_board (std::string config, std::string &response)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    if (config.rfind ("edx:", 0) == 0)
    {
        return parse_edx_command (config, response);
    }
    if (config.find ("sampling_rate:") == 0)
    {
        int value = std::stoi (config.substr (14));
        int res = validate_sampling_rate (value);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        sampling_rate = value;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (config.find ("reference_range:") == 0)
    {
        reference_range = std::stod (config.substr (16));
        int res = validate_reference_range (reference_range);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (config.find ("bipolar_range:") == 0)
    {
        bipolar_range = std::stod (config.substr (14));
        int res = validate_bipolar_range (bipolar_range);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (config.find ("impedance_mode:") == 0)
    {
        bool mode = false;
        if (!parse_bool_flag (config.substr (15), mode))
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        if (impedance_mode == mode)
        {
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        impedance_mode = mode;
        return apply_mode_change ();
    }
    if (config == "get_info")
    {
        json info;
        info["endpoint"] = endpoint;
        info["master_board"] = requested_master_board;
        info["sampling_rate"] = sampling_rate;
        info["selected_model"] = selected_model;
        info["selected_key"] = selected_device_key;
        info["selected_serial"] = selected_device_serial;
        info["impedance_mode"] = impedance_mode;
        info["impedance_sample_count"] = impedance_sample_count;
        info["timing"] = {
            {"missing_start_frame_count", missing_start_frame_count},
            {"fallback_timestamp_count", fallback_timestamp_count},
            {"non_monotonic_timestamp_count", non_monotonic_timestamp_count},
            {"large_gap_count", large_gap_count},
            {"last_emitted_timestamp", last_emitted_timestamp}};
        response = info.dump ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

#else

#include "brainflow_constants.h"

AntNeuroEdxBoard::AntNeuroEdxBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
}

AntNeuroEdxBoard::~AntNeuroEdxBoard ()
{
}

int AntNeuroEdxBoard::prepare_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::start_stream (int, const char *)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::stop_stream ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::release_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::config_board (std::string, std::string &)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::validate_master_board ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::ensure_connected ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::set_mode ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::configure_stream_params (void *)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::process_frames ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroEdxBoard::parse_edx_command (const std::string &, std::string &)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

void AntNeuroEdxBoard::read_thread ()
{
}

bool AntNeuroEdxBoard::parse_bool_flag (const std::string &, bool &)
{
    return false;
}

#endif
