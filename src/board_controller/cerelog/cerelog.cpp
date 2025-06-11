#include <stdint.h>
#include <ctime>
#include "cerelog.h"

#ifndef _WIN32
    #include <errno.h>
#endif

// Constructor
Cerelog_X8::Cerelog_X8(int board_id, struct BrainFlowInputParams params) : Board(board_id, params) {
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    sync_established = false;
    first_packet_counter = 0;
    first_packet_timestamp = 0.0;
    last_sync_counter = 0;
    last_sync_timestamp = 0.0;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    
    /* Set sampling_rate from params
    if (params.other_info.empty()) {
        sampling_rate = 500.0; // default if not provided
    } else {
        try {
            sampling_rate = std::stod(params.other_info);
        } catch (...) {
            sampling_rate = 500.0;
        }
    } */
}

/* This is to configure serial port that Cerelog X8 will use */
int Cerelog_X8::prepare_session() {
    // add error handling later
    serial = Serial::create (params.serial_port.c_str(), this);
    int response = serial->open_serial_port();

    if (response < 0) {
        return 7;
    }

    response = serial->set_serial_port_settings(params.timeout * 1000, false); // this function should be called set_timeout
    response = serial->set_custom_baudrate(921600);

    /* std::string sample_rate = "~250";
    std::string tmp;
    std::string default_settings = "o";
    response = config_board(default_settings, tmp);
    response = config_board(sample_rate, tmp); */   
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

/* This sends config bytes to the message server on the cerelog X8 and it then packages these configurations and executes them */
int Cerelog_X8::config_board(std::string config, std::string &response) {
    // are we sending serial messages back in? Are we sending TCP messages in?
    return 2;
    // we have to build a big list of configurables. How do we ensure that the board is ready to be configured though?
    // Looks like our arduino setup is not sufficient
}

/* This function seems a bit unnecessary for Cerelog since we start barragging with data anyway
   This function is also going to enable timestamp syncing?? 
   This function CALLS READ_THREAD*/
int Cerelog_X8::start_stream(int buffer_size, const char* streamer_params) {
    if (!initialized) {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    if (is_streaming) {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    /* auto syncTimestamp = []() {
        std::string resp;
        for (int i = 0; i < 3; i++) {
            Implement the following
            int res = calc_time(resp);
            if (res != (int)BrainFlowExitCodes::STATUS_OK) {
                return res;
            }
        }
    }; 
*/

    int res = prepare_for_acquisition(buffer_size, streamer_params); // this is brainflow command
    if (res != (int)BrainFlowExitCodes::STATUS_OK) { // check often that the board is ready to do this
        return res;
    }

    // Streaming begins now
    //res = serial->send_to_serial_port("b\n", 2); // We would switch up this message a bit. We weren't thinking about a back and forth paradigm for the board but that is the smart way to do this
    keep_alive = true;
    streaming_thread = std::thread ([this] {this->read_thread();}); //
    safe_logger(spdlog::level::debug, "before mutex shit");
    
    // check for incoming data, wait for 5 seconds
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds(1);
    bool state_changed = cv.wait_for(lk, 5 * sec,
    
    [this]() {
        if (this->state == (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR) {
            safe_logger(spdlog::level::warn, "SYNC_TIMEOUT_ERROR detected in wait_for lambda");
        }
        return (this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR);
    }
    
    );

    if (state_changed) {
        this->is_streaming = true;
        safe_logger(spdlog::level::debug, "THE STATE OF BOARD HAS CHANEGD FROM TIMEOUT ERROR");
        return this->state;
    }
    
    else {
        // Timeout occurred - clean up and return error
        safe_logger(spdlog::level::warn, "Board timed out - stopping thread and cleaning up");
        this->keep_alive = false; // Stop the read thread
        if (streaming_thread.joinable()) {
            streaming_thread.join(); // Wait for thread to finish
        }
        this->is_streaming = false; // Ensure streaming flag is false
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        }
}

/* This is the function that is reading the serial data thread */
void Cerelog_X8::read_thread() {
    // Constants for packet structure
    constexpr int START_MARKER = 0xABCD;
    constexpr int END_MARKER = 0xDCBA;
    constexpr int PACKET_IDX_START_MARKER = 0;
    constexpr int PACKET_IDX_LENGTH = 2;
    constexpr int PACKET_IDX_TIMESTAMP = 3;
    constexpr int PACKET_IDX_ADS1299_DATA = 7;
    constexpr int ADS1299_TOTAL_DATA_BYTES = 27;
    constexpr int PACKET_IDX_CHECKSUM = PACKET_IDX_ADS1299_DATA + ADS1299_TOTAL_DATA_BYTES;
    constexpr int PACKET_IDX_END_MARKER = PACKET_IDX_CHECKSUM + 1;
    constexpr int PACKET_MSG_LENGTH = 4 + ADS1299_TOTAL_DATA_BYTES; // timestamp + data
    constexpr int PACKET_TOTAL_SIZE = 2 + 1 + PACKET_MSG_LENGTH + 1 + 2; // start + len + msg + checksum + end

    // Null/validity checks for serial
    if (serial == nullptr) {
        safe_logger(spdlog::level::err, "Serial pointer is null in read_thread");
        return;
    }

    // Check board_descr and its keys
    if (board_descr.find("default") == board_descr.end()) {
        safe_logger(spdlog::level::err, "Board descriptor 'default' not found");
        return;
    }
    const auto& default_descr = board_descr["default"];
    if (default_descr.find("num_rows") == default_descr.end() ||
        default_descr.find("eeg_channels") == default_descr.end() ||
        default_descr.find("timestamp_channel") == default_descr.end() ||
        default_descr.find("marker_channel") == default_descr.end()) {
        safe_logger(spdlog::level::err, "Board descriptor missing required fields");
        return;
    }

    int num_rows = 0;
    try {
        num_rows = default_descr["num_rows"];
    } catch (...) {
        safe_logger(spdlog::level::err, "Failed to get num_rows from board_descr");
        return;
    }

    std::vector<int> eeg_channels;
    try {
        eeg_channels = default_descr["eeg_channels"].get<std::vector<int>>();
    } catch (...) {
        safe_logger(spdlog::level::err, "Failed to get eeg_channels from board_descr");
        return;
    }

    int timestamp_channel = 0;
    int marker_channel = 0;
    try {
        timestamp_channel = default_descr["timestamp_channel"];
        marker_channel = default_descr["marker_channel"];
    } catch (...) {
        safe_logger(spdlog::level::err, "Failed to get timestamp_channel or marker_channel from board_descr");
        return;
    }

    // Validate channel indices
    if (timestamp_channel < 0 || timestamp_channel >= num_rows ||
        marker_channel < 0 || marker_channel >= num_rows) {
        safe_logger(spdlog::level::err, "Invalid timestamp or marker channel index");
        return;
    }
    if (eeg_channels.size() < 8) {
        safe_logger(spdlog::level::err, "Not enough EEG channels in board_descr (need at least 8)");
        return;
    }
    for (size_t i = 0; i < 8; ++i) {
        if (eeg_channels[i] < 0 || eeg_channels[i] >= num_rows) {
            safe_logger(spdlog::level::err, "EEG channel index {} out of bounds", eeg_channels[i]);
            return;
        }
    }

    // Use RAII for packet buffer
    std::vector<unsigned char> packet(PACKET_TOTAL_SIZE, 0);

    // Use RAII for package buffer
    std::vector<double> package(num_rows, 0.0);

    // New implementation: read 37*2 bytes at a time, search for start marker, sync timestamp, parse, push
    constexpr int READ_CHUNK_SIZE = PACKET_TOTAL_SIZE * 2; // 37*2 = 74 bytes
    std::vector<unsigned char> read_buffer(READ_CHUNK_SIZE, 0);
    int buffer_pos = 0;
    int buffer_len = 0;

    // For timestamp sync
    double last_board_timestamp = 0;
    double last_system_time = 0;
    double timestamp_offset = 0;
    bool timestamp_synced = false;

    while (keep_alive) {
        // Shift any leftover bytes to the start of the buffer
        if (buffer_pos < buffer_len) {
            int leftover = buffer_len - buffer_pos;
            if (leftover > 0 && leftover <= READ_CHUNK_SIZE) {
                std::memmove(read_buffer.data(), read_buffer.data() + buffer_pos, leftover);
                buffer_len = leftover;
            } else {
                buffer_len = 0;
            }
        } else {
            buffer_len = 0;
        }
        buffer_pos = 0;

        // Read more bytes to fill the buffer
        int to_read = READ_CHUNK_SIZE - buffer_len;
        if (to_read <= 0 || (buffer_len < 0) || (buffer_len > READ_CHUNK_SIZE)) {
            safe_logger(spdlog::level::err, "Buffer length invalid: {}", buffer_len);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        int res = serial->read_from_serial_port(read_buffer.data() + buffer_len, to_read);
        if (res > 0) {
            buffer_len += res;
            if (buffer_len > READ_CHUNK_SIZE) {
                safe_logger(spdlog::level::err, "Buffer overflow detected in read_thread");
                buffer_len = READ_CHUNK_SIZE;
            }
        } else {
            safe_logger(spdlog::level::debug, "Failed to read from serial port");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // Scan for start marker in the buffer
        while (buffer_pos + PACKET_TOTAL_SIZE <= buffer_len) {
            // Check for start marker (big endian)
            if (read_buffer[buffer_pos] == ((START_MARKER >> 8) & 0xFF) &&
                read_buffer[buffer_pos + 1] == (START_MARKER & 0xFF)) {

                // Potential packet found, check end marker
                int end_idx = buffer_pos + PACKET_IDX_END_MARKER;
                if (end_idx + 1 >= buffer_len) {
                    // Not enough data for full packet, break to read more
                    break;
                }
                if (read_buffer[end_idx] != ((END_MARKER >> 8) & 0xFF) ||
                    read_buffer[end_idx + 1] != (END_MARKER & 0xFF)) {
                    safe_logger(spdlog::level::warn, "End marker mismatch in buffer scan");
                    buffer_pos += 1;
                    continue;
                }

                // Check checksum
                uint8_t checksum = 0;
                for (int i = PACKET_IDX_LENGTH; i < PACKET_IDX_CHECKSUM; i++) {
                    checksum += read_buffer[buffer_pos + i];
                }
                if (read_buffer[buffer_pos + PACKET_IDX_CHECKSUM] != checksum) {
                    safe_logger(spdlog::level::warn, "Checksum mismatch in buffer scan");
                    buffer_pos += 1;
                    continue;
                }

                // Copy packet to local buffer for parsing
                std::memcpy(packet.data(), read_buffer.data() + buffer_pos, PACKET_TOTAL_SIZE);

                // Parse timestamp (4 bytes, big endian)
                if (PACKET_IDX_TIMESTAMP + 3 >= (int)packet.size()) {
                    safe_logger(spdlog::level::err, "Packet too short for timestamp");
                    buffer_pos += 1;
                    continue;
                }
                uint32_t board_timestamp = 0;
                board_timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP] << 24;
                board_timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP + 1] << 16;
                board_timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP + 2] << 8;
                board_timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP + 3];

                // Timestamp sync logic (BrainFlow style)
                double system_time = time(nullptr);
                if (!timestamp_synced) {
                    // On first valid packet, sync offset
                    timestamp_offset = system_time - (double)board_timestamp;
                    timestamp_synced = true;
                    last_board_timestamp = (double)board_timestamp;
                    last_system_time = system_time;
                } else {
                    // If board timestamp jumps, resync
                    if (std::abs((double)board_timestamp - last_board_timestamp) > 10000) {
                        timestamp_offset = system_time - (double)board_timestamp;
                        safe_logger(spdlog::level::warn, "Timestamp jump detected, resyncing offset");
                    }
                    last_board_timestamp = (double)board_timestamp;
                    last_system_time = system_time;
                }
                double synced_timestamp = (double)board_timestamp + timestamp_offset;

                // Parse ADS1299 data (27 bytes, 8 channels, 3 bytes per channel)
                for (int ch = 0; ch < 8; ch++) {
                    int idx = PACKET_IDX_ADS1299_DATA + ch * 3;
                    if (idx + 2 >= (int)packet.size()) {
                        safe_logger(spdlog::level::err, "Packet too short for ADS1299 data, channel {}", ch);
                        continue;
                    }
                    int32_t value = ((int32_t)packet[idx] << 16) |
                                    ((int32_t)packet[idx + 1] << 8) |
                                    ((int32_t)packet[idx + 2]);
                    // Sign extension for 24-bit
                    if (value & 0x800000) value |= 0xFF000000;
                    // Convert to microvolts and store in correct channel
                    double microvolts = (double)value * (4.5 / ((1 << 23) - 1));
                    if (eeg_channels[ch] >= 0 && eeg_channels[ch] < num_rows) {
                        package[eeg_channels[ch]] = microvolts;
                    } else {
                        safe_logger(spdlog::level::warn, "EEG channel index {} out of bounds for package", eeg_channels[ch]);
                    }
                }

                // Add synced timestamp
                if (timestamp_channel >= 0 && timestamp_channel < num_rows)
                    package[timestamp_channel] = synced_timestamp;
                else
                    safe_logger(spdlog::level::warn, "Timestamp channel index {} out of bounds for package", timestamp_channel);

                // Add marker (default to 0)
                if (marker_channel >= 0 && marker_channel < num_rows)
                    package[marker_channel] = 0.0;
                else
                    safe_logger(spdlog::level::warn, "Marker channel index {} out of bounds for package", marker_channel);

                // Push the package to the buffer
                if (!package.empty() && (int)package.size() == num_rows) {
                    push_package(package.data());
                } else {
                    safe_logger(spdlog::level::err, "Package size mismatch or empty package");
                }

                // Set state and notify if first package
                if (this->state != (int)BrainFlowExitCodes::STATUS_OK) {
                    safe_logger(spdlog::level::info, "received first package streaming is started");
                    {
                        std::lock_guard<std::mutex> lk(this->m);
                        this->state = (int)BrainFlowExitCodes::STATUS_OK;
                    }
                    this->cv.notify_one();
                    safe_logger(spdlog::level::info, "start streaming");
                }

                buffer_pos += PACKET_TOTAL_SIZE;
            } else {
                // Not a start marker, move to next byte
                buffer_pos += 1;
            }
        }
        // If keep_alive is false, break out
        if (!keep_alive) break;
    }
}

/* This will send a message to stop sending data
It creates a spare buffer to read any spare bytes coming in from the board.
If the board keep sending more and more, up to the limit of 40000 bytes, send an error message */
/* int Cerelog_X8::stop_stream() {
    if (is_streaming) {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join(); //no idea what this does
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        // Now send the STOP_STREAMING command to the board
        // clear the serial buffer
        unsigned char buffer;
        int res = 1;
        int max_attempts = 400000;
        int cur_attempts = 0;
        while (res == 1) {
            res = serial->read_from_serial_port(&buffer, 1);
            cur_attempts++;

            if (cur_attempts == max_attempts) {
                safe_logger(spdlog::level::err, "We told it to stop but it no listen bruv.");
                return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
            }
        }
    }

    return 3;
} */

int Cerelog_X8::stop_stream() {
    // turn off flow
    safe_logger(spdlog::level::err, "STOP STREAM FUNCTION CALLED BABY");
    if (is_streaming) {
        keep_alive = false;
        is_streaming = false;
        if (streaming_thread.joinable()) {
            streaming_thread.join();
        }

        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    else {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }

}

/* This function calls the stop_stream() function */
int Cerelog_X8::release_session() {
    if (initialized) {
        if (is_streaming) {
            this->stop_stream(); // Use Cerelog_X8's stop_stream() method
        }

        free_packages();
        initialized = false;
        if (serial) {
            delete serial;
            serial = NULL;
        }
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Cerelog_X8::config_board_with_bytes(const char* bytes, int len) {
    return 5;
}

double Cerelog_X8::convert_counter_to_timestamp(uint64_t packet_counter) {
    if (!Cerelog_X8::sync_established) {
        first_packet_counter = packet_counter;
        first_packet_timestamp = time(nullptr); // is this helper function activated
        Cerelog_X8::sync_established = true;
        Cerelog_X8::last_sync_counter = packet_counter;
        Cerelog_X8::last_sync_timestamp = first_packet_timestamp;
        return first_packet_timestamp;
    }

    return 3;
}