#include "Wire.h"

#include <cstring>
#include <kvn/kvn_bytearray.h>
#include <fmt/core.h>

using namespace SimpleBLE::Dongl::Serial;

Wire::Wire(const std::string& device_path)
    : _usb_helper(std::make_unique<USB::UsbHelper>(device_path))
    , _state(State::IDLE)
    , _length(0)
    , _checksum(0)
    , _buffer_index(0)
    , _buffer(MAX_PAYLOAD_SIZE) {

    // Set up the USB receive callback to process incoming bytes
    _usb_helper->set_rx_callback([this](const kvn::bytearray& data) {
        for (size_t i = 0; i < data.size(); ++i) {
            process_byte(data[i]);
        }
    });
}

Wire::~Wire() {}

void Wire::send_packet(const std::vector<uint8_t>& payload) {
    send_packet(payload.data(), payload.size());
}

void Wire::send_packet(const uint8_t* data, size_t length) {
    if (length > MAX_PAYLOAD_SIZE) {
        throw std::runtime_error("Payload length exceeds maximum allowed");
    }

    // Create the encoded packet
    std::vector<uint8_t> packet;
    packet.reserve(5 + length);  // sync + length(2) + payload + crc(2)

    packet.push_back(SYNC_BYTE);
    packet.push_back(length & 0xFF);
    packet.push_back((length >> 8) & 0xFF);
    packet.insert(packet.end(), data, data + length);

    uint16_t crc = crc16(data, length);
    packet.push_back(crc & 0xFF);
    packet.push_back((crc >> 8) & 0xFF);

    // Send via USB
    kvn::bytearray usb_data(packet.data(), packet.size());
    _usb_helper->tx(usb_data);
}

void Wire::set_packet_callback(PacketCallback callback) {
    _packet_callback = std::move(callback);
}

void Wire::set_error_callback(ErrorCallback callback) {
    _error_callback = std::move(callback);
}

void Wire::process_byte(uint8_t byte) {
    switch (_state) {
        case State::IDLE:
            if (byte == SYNC_BYTE) {
                _state = State::HEADER_LOW;
                _buffer_index = 0;
            }
            break;

        case State::HEADER_LOW:
            _length = byte;
            _state = State::HEADER_HIGH;
            break;

        case State::HEADER_HIGH:
            _length |= (uint16_t)byte << 8;
            if (_length > MAX_PAYLOAD_SIZE) {
                _state = State::IDLE;
                if (_error_callback) {
                    _error_callback(Error::INVALID_LENGTH);
                }
            } else if (_length == 0) {
                _state = State::CHECKSUM_LOW;
                _buffer_index = 0;
            } else {
                _state = State::PAYLOAD;
                _buffer_index = 0;
            }
            break;

        case State::PAYLOAD:
            _buffer[_buffer_index++] = byte;
            if (_buffer_index == _length) {
                _state = State::CHECKSUM_LOW;
            }
            break;

        case State::CHECKSUM_LOW:
            _checksum = byte;
            _state = State::CHECKSUM_HIGH;
            break;

        case State::CHECKSUM_HIGH:
            _checksum |= (uint16_t)byte << 8;
            uint16_t computed_crc = crc16(_buffer.data(), _length);
            if (_checksum == computed_crc) {
                if (_packet_callback) {
                    std::vector<uint8_t> packet(_buffer.begin(), _buffer.begin() + _length);
                    _packet_callback(packet);
                }
            } else if (_error_callback) {
                _error_callback(Error::CRC_FAILURE);
            }
            _state = State::IDLE;
            break;
    }
}

uint16_t Wire::crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x8005;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
