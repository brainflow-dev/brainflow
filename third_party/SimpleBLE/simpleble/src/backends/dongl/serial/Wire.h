#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "../usb/UsbHelper.h"

namespace SimpleBLE {
namespace Dongl {
namespace Serial {

class Wire {
public:
    /**
     * @brief Error codes for the serial protocol.
     */
    enum class Error {
        INVALID_LENGTH, /**< Length exceeds maximum payload size. */
        CRC_FAILURE     /**< CRC-16 check failed. */
    };

    /**
     * @brief State machine states.
     */
    enum class State {
        IDLE,         /**< Waiting for sync byte. */
        HEADER_LOW,   /**< Reading low byte of length. */
        HEADER_HIGH,  /**< Reading high byte of length. */
        PAYLOAD,      /**< Reading payload bytes. */
        CHECKSUM_LOW, /**< Reading low byte of CRC-16. */
        CHECKSUM_HIGH /**< Reading high byte of CRC-16. */
    };

    /**
     * @brief Callback function type for received packets.
     */
    using PacketCallback = std::function<void(const std::vector<uint8_t>&)>;

    /**
     * @brief Callback function type for errors.
     */
    using ErrorCallback = std::function<void(Error)>;

    /**
     * @brief Maximum payload size in bytes.
     */
    static constexpr size_t MAX_PAYLOAD_SIZE = 571;

    /**
     * @brief Sync byte value marking packet start.
     */
    static constexpr uint8_t SYNC_BYTE = 0xAA;

    Wire(const std::string& device_path);
    ~Wire();

    /**
     * @brief Sends a payload as an encoded packet.
     *
     * @param payload The data to send.
     */
    void send_packet(const std::vector<uint8_t>& payload);

    /**
     * @brief Sends a payload as an encoded packet.
     *
     * @param data Pointer to the data to send.
     * @param length Length of the data.
     */
    void send_packet(const uint8_t* data, size_t length);

    /**
     * @brief Sets the callback for received packets.
     *
     * @param callback Function to call when a valid packet is received.
     */
    void set_packet_callback(PacketCallback callback);

    /**
     * @brief Sets the callback for protocol errors.
     *
     * @param callback Function to call on protocol errors.
     */
    void set_error_callback(ErrorCallback callback);

private:
    /**
     * @brief Processes a single incoming byte through the protocol state machine.
     *
     * @param byte The incoming byte to process.
     */
    void process_byte(uint8_t byte);

    /**
     * @brief Computes CRC-16 checksum.
     *
     * @param data Pointer to the data.
     * @param len Length of the data.
     * @return The computed CRC-16 value.
     */
    static uint16_t crc16(const uint8_t* data, size_t len);

    std::unique_ptr<USB::UsbHelper> _usb_helper;
    State _state;
    uint16_t _length;
    uint16_t _checksum;
    size_t _buffer_index;
    std::vector<uint8_t> _buffer;
    PacketCallback _packet_callback;
    ErrorCallback _error_callback;
};

}  // namespace Serial
}  // namespace Dongl
}  // namespace SimpleBLE
