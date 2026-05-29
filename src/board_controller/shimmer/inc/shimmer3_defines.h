#pragma once

#include <cstdint>
#include <cstring>

namespace shimmer3
{

    // Hardware version reported by GET_SHIMMER_VERSION_COMMAND. We only accept 3.
    constexpr uint8_t HW_VERSION_SHIMMER3 = 3;
    constexpr uint8_t HW_VERSION_SHIMMER3R = 10; // listed only so we can reject it

    // The Shimmer3's low-frequency crystal runs at 32768 Hz. Sampling rate is
    // stored on the device as a clock divider: f = 32768 / divider.
    constexpr double CLOCK_HZ = 32768.0;

    // Width of the on-wire timestamp that leads every data packet.
    constexpr int TIMESTAMP_BYTES = 3;

    // Bluetooth command / response opcodes (subset that this driver uses).
    enum Opcode : uint8_t
    {
        DATA_PACKET = 0x00,

        INQUIRY_COMMAND = 0x01,
        INQUIRY_RESPONSE = 0x02,

        GET_SAMPLING_RATE_COMMAND = 0x03,
        SAMPLING_RATE_RESPONSE = 0x04,
        SET_SAMPLING_RATE_COMMAND = 0x05,

        START_STREAMING_COMMAND = 0x07,
        SET_SENSORS_COMMAND = 0x08,

        STOP_STREAMING_COMMAND = 0x20,

        DEVICE_VERSION_RESPONSE = 0x25,

        GET_ALL_CALIBRATION_COMMAND = 0x2C,
        ALL_CALIBRATION_RESPONSE = 0x2D,

        GET_FW_VERSION_COMMAND = 0x2E,
        FW_VERSION_RESPONSE = 0x2F,

        GET_SHIMMER_VERSION_COMMAND = 0x3F,

        SET_EXG_REGS_COMMAND = 0x61,
        EXG_REGS_RESPONSE = 0x62,
        GET_EXG_REGS_COMMAND = 0x63,

        START_SDBT_COMMAND = 0x70, // start streaming AND logging to SD
        STATUS_RESPONSE = 0x71,
        GET_STATUS_COMMAND = 0x72,

        SET_SHIMMERNAME_COMMAND = 0x79,
        SHIMMERNAME_RESPONSE = 0x7A,
        GET_SHIMMERNAME_COMMAND = 0x7B,

        INSTREAM_CMD_RESPONSE = 0x8A,
        SET_CRC_COMMAND = 0x8B,

        VBATT_RESPONSE = 0x94,
        GET_VBATT_COMMAND = 0x95,
        TEST_CONNECTION_COMMAND = 0x96,

        SET_INSTREAM_RESPONSE_ACK_PREFIX_STATE = 0xA3,

        ACK_COMMAND_PROCESSED = 0xFF,
    };

    // 24-bit "enabled sensors" bitfield used by SET_SENSORS_COMMAND.
    // The field is transmitted little-endian: byte0 = (field & 0xFF), etc.
    namespace sensor
    {
        constexpr uint32_t A_ACCEL = 0x000080;    // low-noise (analog) accel
        constexpr uint32_t GYRO = 0x000040;       // MPU9150 gyroscope
        constexpr uint32_t MAG = 0x000020;        // LSM303 magnetometer
        constexpr uint32_t EXG1_24BIT = 0x000010; // ADS1292R chip 1, 24-bit
        constexpr uint32_t EXG2_24BIT = 0x000008; // ADS1292R chip 2, 24-bit
        constexpr uint32_t GSR = 0x000004;
        constexpr uint32_t EXT_A1 = 0x000002; // external ADC
        constexpr uint32_t EXT_A0 = 0x000001; // external ADC

        constexpr uint32_t INT_A1 = 0x000100;  // internal ADC
        constexpr uint32_t INT_A0 = 0x000200;  // internal ADC
        constexpr uint32_t INT_A3 = 0x000400;  // internal ADC
        constexpr uint32_t EXT_A2 = 0x000800;  // external ADC
        constexpr uint32_t D_ACCEL = 0x001000; // wide-range (digital) accel
        constexpr uint32_t VBATT = 0x002000;
        constexpr uint32_t STRAIN = 0x008000;

        constexpr uint32_t TEMP = 0x020000;
        constexpr uint32_t PRESSURE = 0x040000;
        constexpr uint32_t EXG2_16BIT = 0x080000;
        constexpr uint32_t EXG1_16BIT = 0x100000;
        constexpr uint32_t MAG_WR = 0x200000;
        constexpr uint32_t HIGH_G_ACCEL = 0x400000;
        constexpr uint32_t INT_A2 = 0x800000;
    }

    // Channel (signal) identifiers as reported, in order, by the inquiry response.
    // These tell us how to lay out the bytes that follow the timestamp.
    enum class Signal : uint8_t
    {
        ACCEL_LN_X = 0x00,
        ACCEL_LN_Y = 0x01,
        ACCEL_LN_Z = 0x02,
        VBATT = 0x03,
        ACCEL_WR_X = 0x04,
        ACCEL_WR_Y = 0x05,
        ACCEL_WR_Z = 0x06,
        MAG_X = 0x07,
        MAG_Y = 0x08,
        MAG_Z = 0x09,
        GYRO_X = 0x0A,
        GYRO_Y = 0x0B,
        GYRO_Z = 0x0C,
        EXT_ADC_A0 = 0x0D,
        EXT_ADC_A1 = 0x0E,
        EXT_ADC_A2 = 0x0F,
        INT_ADC_A3 = 0x10,
        INT_ADC_A0 = 0x11,
        INT_ADC_A1 = 0x12,
        INT_ADC_A2 = 0x13,
        HIGH_G_ACCEL_X = 0x14,
        HIGH_G_ACCEL_Y = 0x15,
        HIGH_G_ACCEL_Z = 0x16,
        MAG_WR_X = 0x17,
        MAG_WR_Y = 0x18,
        MAG_WR_Z = 0x19,
        TEMPERATURE = 0x1A,
        PRESSURE = 0x1B,
        GSR = 0x1C,
        EXG1_STATUS = 0x1D,
        EXG1_CH1_24BIT = 0x1E,
        EXG1_CH2_24BIT = 0x1F,
        EXG2_STATUS = 0x20,
        EXG2_CH1_24BIT = 0x21,
        EXG2_CH2_24BIT = 0x22,
        EXG1_CH1_16BIT = 0x23,
        EXG1_CH2_16BIT = 0x24,
        EXG2_CH1_16BIT = 0x25,
        EXG2_CH2_16BIT = 0x26,
        STRAIN_HIGH = 0x27,
        STRAIN_LOW = 0x28,

        // Synthetic: the 3-byte timestamp prepended to every packet. Not a real
        // signal ID, so we use a value outside the on-wire 8-bit range conceptually
        // (kept inside uint8_t but never sent by the device).
        TIMESTAMP = 0xFE,
    };

    // Wire format of a single field inside a data packet.
    struct FieldFormat
    {
        uint8_t width;  // 1, 2 or 3 bytes
        bool is_signed; // sign-extend when decoding
        bool little_endian;
    };

    // Decode a field's raw bytes into a signed 32-bit integer.
    inline int32_t decode_field (const FieldFormat &fmt, const uint8_t *p)
    {
        uint32_t raw = 0;
        if (fmt.little_endian)
        {
            for (int i = fmt.width - 1; i >= 0; --i)
                raw = (raw << 8) | p[i];
        }
        else
        {
            for (int i = 0; i < fmt.width; ++i)
                raw = (raw << 8) | p[i];
        }

        if (fmt.is_signed && fmt.width < 4)
        {
            const uint32_t sign_bit = 1u << (fmt.width * 8 - 1);
            if (raw & sign_bit)
                raw |= ~((1u << (fmt.width * 8)) - 1);
        }
        return static_cast<int32_t> (raw);
    }

    // Return the wire format for a given signal. found is set false for signals
    // that exist in the protocol but are not produced by Shimmer3 hardware.
    inline FieldFormat format_for (Signal s, bool &found)
    {
        found = true;
        switch (s)
        {
            // Low-noise analog accel + battery: unsigned 16-bit little-endian ADC.
            case Signal::ACCEL_LN_X:
            case Signal::ACCEL_LN_Y:
            case Signal::ACCEL_LN_Z:
            case Signal::VBATT:
                return {2, false, true};

            // Wide-range (LSM303) accel: signed 16-bit little-endian.
            case Signal::ACCEL_WR_X:
            case Signal::ACCEL_WR_Y:
            case Signal::ACCEL_WR_Z:
                return {2, true, true};

            // LSM303 magnetometer: signed 16-bit big-endian.
            case Signal::MAG_X:
            case Signal::MAG_Y:
            case Signal::MAG_Z:
                return {2, true, false};

            // MPU9150 gyroscope: signed 16-bit big-endian.
            case Signal::GYRO_X:
            case Signal::GYRO_Y:
            case Signal::GYRO_Z:
                return {2, true, false};

            // ADC channels: unsigned 16-bit little-endian.
            case Signal::EXT_ADC_A0:
            case Signal::EXT_ADC_A1:
            case Signal::EXT_ADC_A2:
            case Signal::INT_ADC_A3:
            case Signal::INT_ADC_A0:
            case Signal::INT_ADC_A1:
            case Signal::INT_ADC_A2:
            case Signal::GSR:
            case Signal::STRAIN_HIGH:
            case Signal::STRAIN_LOW:
                return {2, false, true};

            // BMP180/280 temperature & pressure: unsigned big-endian.
            case Signal::TEMPERATURE:
                return {2, false, false};
            case Signal::PRESSURE:
                return {3, false, false};

            // ExG (ADS1292R): 1-byte status, 24-/16-bit signed big-endian data.
            case Signal::EXG1_STATUS:
            case Signal::EXG2_STATUS:
                return {1, false, true};
            case Signal::EXG1_CH1_24BIT:
            case Signal::EXG1_CH2_24BIT:
            case Signal::EXG2_CH1_24BIT:
            case Signal::EXG2_CH2_24BIT:
                return {3, true, false};
            case Signal::EXG1_CH1_16BIT:
            case Signal::EXG1_CH2_16BIT:
            case Signal::EXG2_CH1_16BIT:
            case Signal::EXG2_CH2_16BIT:
                return {2, true, false};

            case Signal::TIMESTAMP:
                return {TIMESTAMP_BYTES, false, true};

            // Not present on Shimmer3 (high-g accel, wide-range mag).
            default:
                found = false;
                return {0, false, false};
        }
    }

    // ---- Sampling-rate conversion (f = 32768 / divider) ----

    inline double divider_to_hz (uint16_t divider)
    {
        return divider == 0 ? 0.0 : CLOCK_HZ / static_cast<double> (divider);
    }

    inline uint16_t hz_to_divider (double hz)
    {
        if (hz <= 0.0)
            return 0;
        return static_cast<uint16_t> (CLOCK_HZ / hz + 0.5);
    }

    // -------------------------- CRC --------------------------
    // Optional: only needed if SET_CRC_COMMAND is used to enable BT-packet CRC.

    constexpr uint16_t CRC_INIT = 0xB0CA;

    inline uint16_t crc_byte (uint16_t crc, uint8_t b)
    {
        crc = static_cast<uint16_t> (((crc >> 8) & 0xFFFF) | ((crc << 8) & 0xFFFF));
        crc ^= b;
        crc ^= (crc & 0xFF) >> 4;
        crc = static_cast<uint16_t> (crc ^ ((crc << 12) & 0xFFFF));
        crc = static_cast<uint16_t> (crc ^ ((crc & 0xFF) << 5));
        return crc;
    }

    inline uint16_t calc_crc (int length, const uint8_t *msg)
    {
        uint16_t crc = crc_byte (CRC_INIT, msg[0]);
        for (int i = 1; i < length; ++i)
            crc = crc_byte (crc, msg[i]);
        if (length % 2 == 1)
            crc = crc_byte (crc, 0x00);
        return crc;
    }

} // namespace shimmer3
