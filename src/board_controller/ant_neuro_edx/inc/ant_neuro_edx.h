#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "board.h"

#ifdef BUILD_ANT_EDX
#include <grpcpp/grpcpp.h>

#include "EdigRPC.grpc.pb.h"
#endif


// Mode-change requests processed by the read_thread between gRPC frames.
// The read_thread is the only safe place to call Amplifier_SetMode because the
// EDX gRPC server corrupts its internal state if SetMode races with GetFrame.
enum class EdxModeRequest : int
{
    None = 0,       // No pending request
    Idle = 1,       // Transition to idle mode
    Eeg = 2,        // Transition to EEG streaming mode
    Impedance = 3   // Transition to impedance mode
};


class AntNeuroEdxBoard : public Board
{
private:
#ifdef BUILD_ANT_EDX
    struct EdxChannelMeta
    {
        int index;
        EdigRPC::gen::ChannelPolarity polarity;
        std::string name;
    };
#endif

    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    std::mutex wait_mutex;
    std::condition_variable wait_cv;
    volatile int state;

    // Atomic mode-change state machine.
    // Callers set mode_request, the read_thread processes it between frames
    // and writes the result to mode_result, then notifies via mode_cv.
    std::atomic<EdxModeRequest> mode_request;
    std::atomic<int> mode_result;
    std::mutex mode_mutex;
    std::condition_variable mode_cv;

    int amplifier_handle;
    int requested_master_board;
    int package_num;
    int sampling_rate;
    double reference_range;
    double bipolar_range;
    bool impedance_mode;
    std::vector<int> active_channel_indices;
    std::string endpoint;
    int trigger_channel_index;
    // Timing diagnostics are exposed via get_info and do not alter stream shape.
    uint64_t missing_start_frame_count;
    uint64_t fallback_timestamp_count;
    uint64_t non_monotonic_timestamp_count;
    uint64_t large_gap_count;
    double last_emitted_timestamp;
    uint64_t impedance_sample_count;
#ifdef BUILD_ANT_EDX
    grpc::ClientContext *volatile streaming_context;
    std::shared_ptr<grpc::Channel> grpc_channel;
    std::unique_ptr<EdigRPC::gen::EdigRPC::Stub> stub;
    std::vector<EdxChannelMeta> channel_meta;
    std::vector<int> sampling_rates_available;
    std::vector<double> reference_ranges_available;
    std::vector<double> bipolar_ranges_available;
    std::vector<EdigRPC::gen::AmplifierMode> modes_available;
    std::string selected_model;
    std::string selected_device_key;
    std::string selected_device_serial;
#endif

    int validate_master_board ();
    int ensure_connected ();
    int set_mode ();
    int configure_stream_params (void *request_ptr);
    int process_frames ();
    int apply_mode_change ();
    int parse_edx_command (const std::string &config, std::string &response);
    void read_thread ();
    int request_mode_from_thread (EdxModeRequest request);
    void process_mode_request ();
    bool parse_bool_flag (const std::string &value, bool &flag);
#ifdef BUILD_ANT_EDX
    int connect_and_create_device ();
    int load_capabilities ();
    int set_idle_mode ();
    int validate_sampling_rate (int value);
    int validate_reference_range (double value);
    int validate_bipolar_range (double value);
#endif

public:
    AntNeuroEdxBoard (int board_id, struct BrainFlowInputParams params);
    ~AntNeuroEdxBoard ();

    int prepare_session () override;
    int start_stream (int buffer_size, const char *streamer_params) override;
    int stop_stream () override;
    int release_session () override;
    int config_board (std::string config, std::string &response) override;
};
