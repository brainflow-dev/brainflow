#include <stdint.h>
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
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
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

/* This function seems a bit unnecessary for Cerelog since we start barragging with data anyway */
int Cerelog_X8::start_stream(int buffer_size, const char* streamer_params) {
    if (!initialized) {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    if (is_streaming) {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    auto syncTimestamp = []() {
        std::string resp;
        for (int i = 0; i < 3; i++) {
            /* Implement the following
            int res = calc_time(resp);
            if (res != (int)BrainFlowExitCodes::STATUS_OK) {
                return res;
            } */
        }
    };

    int res = prepare_for_acquisition(buffer_size, streamer_params); // this is brainflow command
    if (res != (int)BrainFlowExitCodes::STATUS_OK) { // check often that the board is ready to do this
        return res;
    }

    // Streaming begins now
    //res = serial->send_to_serial_port("b\n", 2); // We would switch up this message a bit. We weren't thinking about a back and forth paradigm for the board but that is the smart way to do this
    keep_alive = true;
    streaming_thread = std::thread ([this] {this->read_thread();});
    
    // check for incomign data.
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds(1);
    if (cv.wait_for(lk, 5*sec,
        [this] {return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;})) {
            this->is_streaming = true;
            return this->state;
        }
    
    else {
        is_streaming = true;
        stop_stream();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
    
    return 3;
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
    
    unsigned char packet[PACKET_TOTAL_SIZE];
    
    // Get board configuration
    int num_rows = board_descr["default"]["num_rows"];
    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    int timestamp_channel = board_descr["default"]["timestamp_channel"];
    int marker_channel = board_descr["default"]["marker_channel"];
    
    // Initialize package with correct size
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++) {
        package[i] = 0.0;
    }

    while (keep_alive) {
        // Read until we find the start marker
        int res = serial->read_from_serial_port(packet, 1);
        if (res != 1) {
            safe_logger(spdlog::level::debug, "Unable to read 1 byte for start marker");
            continue;
        }
        if (packet[0] != ((START_MARKER >> 8) & 0xFF)) {
            safe_logger(spdlog::level::debug, "Read one byte but not starter marker");

            continue;
        }
        // Read the rest of the start marker
        res = serial->read_from_serial_port(packet + 1, 1);
        if (res != 1 || packet[1] != (START_MARKER & 0xFF)) {
            continue;
        }

        // Read the rest of the packet
        int to_read = PACKET_TOTAL_SIZE - 2;
        int pos = 2;
        while (to_read > 0) {
            res = serial->read_from_serial_port(packet + pos, to_read);
            if (res <= 0) {
                safe_logger(spdlog::level::debug, "Failed to read packet body");
                break;
            }
            pos += res;
            to_read -= res;
        }
        if (to_read > 0) {
            continue; // incomplete packet
        }

        // Check end marker
        if (packet[PACKET_IDX_END_MARKER] != ((END_MARKER >> 8) & 0xFF) ||
            packet[PACKET_IDX_END_MARKER + 1] != (END_MARKER & 0xFF)) {
            safe_logger(spdlog::level::warn, "End marker mismatch");
            continue;
        }

        // Check checksum
        uint8_t checksum = 0;
        for (int i = PACKET_IDX_LENGTH; i < PACKET_IDX_CHECKSUM; i++) {
            checksum += packet[i];
        }
        if (packet[PACKET_IDX_CHECKSUM] != checksum) {
            safe_logger(spdlog::level::warn, "Checksum mismatch");
            continue;
        }

        // Parse timestamp (4 bytes, big endian)
        uint32_t timestamp = 0;
        timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP] << 24;
        timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP + 1] << 16;
        timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP + 2] << 8;
        timestamp |= (uint32_t)packet[PACKET_IDX_TIMESTAMP + 3];

        // Parse ADS1299 data (27 bytes, 8 channels, 3 bytes per channel)
        // Each channel is 3 bytes, signed 24-bit integer
        for (int ch = 0; ch < 8; ch++) {
            int idx = PACKET_IDX_ADS1299_DATA + ch * 3;
            int32_t value = ((int32_t)packet[idx] << 16) |
                           ((int32_t)packet[idx + 1] << 8) |
                           ((int32_t)packet[idx + 2]);
            // Sign extension for 24-bit
            if (value & 0x800000) value |= 0xFF000000;
            
            // Convert to microvolts and store in correct channel
            double microvolts = (double)value * (4.5 / ((1 << 23) - 1));
            package[eeg_channels[ch]] = microvolts;
        }

        // Add timestamp
        package[timestamp_channel] = (double)timestamp;
        
        // Add marker (default to 0)
        package[marker_channel] = 0.0;

        // Push the package to the buffer
        push_package(package);

        safe_logger(spdlog::level::debug, "Package data: timestamp={}, marker={}, eeg_values={:.6f}, {:.6f}, {:.6f}", 
            package[timestamp_channel], package[marker_channel],
            // Log first few EEG values
            package[eeg_channels[0]], package[eeg_channels[1]], package[eeg_channels[2]]);

        // Set state and notify if first package
        if (this->state != (int)BrainFlowExitCodes::STATUS_OK) {
            safe_logger(spdlog::level::info, "received first package streaming is started");
            {
                std::lock_guard<std::mutex> lk(this->m);
                this->state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            this->cv.notify_one();
            safe_logger(spdlog::level::debug, "start streaming");
        }
    }
    
    delete[] package;
}

/* This will send a message to stop sending data
It creates a spare buffer to read any spare bytes coming in from the board.
If the board keep sending more and more, up to the limit of 40000 bytes, send an error message */
int Cerelog_X8::stop_stream() {
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