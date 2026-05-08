#include "ProtocolBase.h"

using namespace SimpleBLE::Dongl::Serial;

#include "nanopb/pb_decode.h"
#include "nanopb/pb_encode.h"

#include <fmt/core.h>

ProtocolBase::ProtocolBase(const std::string& device_path) : _wire(std::make_unique<Wire>(device_path)) {
    // Set up the Wire packet callback to handle incoming packets
    _wire->set_packet_callback([this](const std::vector<uint8_t>& packet) {
        dongl_D2H d2h;
        pb_istream_t stream = pb_istream_from_buffer(packet.data(), packet.size());
        if (!pb_decode(&stream, dongl_D2H_fields, &d2h)) {
            // TODO: Handle decoding failure
            fmt::print("Failed to decode D2H: {}\n", PB_GET_ERROR(&stream));
            return;
        }

        if (d2h.which_type == dongl_D2H_rsp_tag) {
            // First, try to fulfill any pending sync request
            {
                std::lock_guard<std::mutex> lock(_pending_mutex);
                if (!_pending_response.has_value()) {
                    _pending_response = d2h.type.rsp;
                    _response_cv.notify_one();
                }
            }

        } else if (d2h.which_type == dongl_D2H_evt_tag) {
            if (_event_callback) {
                _event_callback(d2h.type.evt);
            }
        }
    });

    _wire->set_error_callback([this](const Wire::Error& error) {
        fmt::print("Error: {}\n", (int)error);
    });
}

ProtocolBase::~ProtocolBase() {}

dongl_Response ProtocolBase::exchange(const dongl_Command& command) {
    // Check if there's already a pending sync operation
    {
        std::lock_guard<std::mutex> lock(_pending_mutex);
        if (_pending_response.has_value()) {
            throw std::runtime_error("Another sync command is already pending");
        }
    }

    // Clear any previous response and send the command
    {
        std::unique_lock<std::mutex> lock(_pending_mutex);
        _pending_response.reset();  // Ensure it's empty to indicate we're waiting
    }

    try {
        size_t command_size = 0;
        pb_get_encoded_size(&command_size, dongl_Command_fields, &command);

        std::vector<uint8_t> tx_buffer_raw(command_size);
        pb_ostream_t stream = pb_ostream_from_buffer(tx_buffer_raw.data(), tx_buffer_raw.size());
        bool status = pb_encode(&stream, dongl_Command_fields, &command);
        if (!status) {
            // TODO: Handle encoding failure
            throw std::runtime_error("Failed to encode command");
        }

        _wire->send_packet(tx_buffer_raw);
    } catch (const std::runtime_error& e) {
        // If sending fails, no cleanup needed since we didn't set pending_response
        throw;
    }

    // Wait for the response with 1 second timeout
    {
        std::unique_lock<std::mutex> lock(_pending_mutex);
        if (_response_cv.wait_for(lock, std::chrono::seconds(1), [this]() { return _pending_response.has_value(); })) {
            dongl_Response response = *_pending_response;
            _pending_response.reset();
            return response;
        } else {
            // Timeout occurred, reset pending state
            _pending_response.reset();
            throw std::runtime_error("Timeout waiting for response");
        }
    }
}

void ProtocolBase::set_event_callback(std::function<void(const dongl_Event&)> callback) {
    _event_callback = std::move(callback);
}
