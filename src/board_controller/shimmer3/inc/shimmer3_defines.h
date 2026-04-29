/*
 * Shimmer3 protocol definitions for BrainFlow.
 *
 * Adapted from the pyshimmer Python library by semoo-lab:
 *   https://github.com/seemoo-lab/pyshimmer
 *
 * Original work licensed under the GNU General Public License v3.0.
 * See https://www.gnu.org/licenses/gpl-3.0.html for details.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <map>
#include <vector>


// Shimmer3 Bluetooth command/response byte constants.
// These are the opcodes used in the binary serial protocol between
// the host and the Shimmer3 over Bluetooth SPP.
namespace ShimmerBT
{
    // Acknowledgment and framing
    constexpr uint8_t ACK_COMMAND_PROCESSED = 0xFF;
    constexpr uint8_t INSTREAM_CMD_RESPONSE = 0x8A;
    constexpr uint8_t DATA_PACKET = 0x00;

    // Inquiry
    constexpr uint8_t INQUIRY_COMMAND = 0x01;
    constexpr uint8_t INQUIRY_RESPONSE = 0x02;

    // Sampling rate
    constexpr uint8_t GET_SAMPLING_RATE_COMMAND = 0x03;
    constexpr uint8_t SAMPLING_RATE_RESPONSE = 0x04;
    constexpr uint8_t SET_SAMPLING_RATE_COMMAND = 0x05;

    // Battery
    constexpr uint8_t GET_BATTERY_COMMAND = 0x95;
    constexpr uint8_t BATTERY_RESPONSE = 0x94;

    // Streaming control
    constexpr uint8_t START_STREAMING_COMMAND = 0x07;
    constexpr uint8_t STOP_STREAMING_COMMAND = 0x20;

    // Sensor selection
    constexpr uint8_t SET_SENSORS_COMMAND = 0x08;

    // Hardware version
    constexpr uint8_t GET_SHIMMER_VERSION_COMMAND = 0x3F;
    constexpr uint8_t SHIMMER_VERSION_RESPONSE = 0x25;

    // Config time
    constexpr uint8_t GET_CONFIGTIME_COMMAND = 0x87;
    constexpr uint8_t CONFIGTIME_RESPONSE = 0x86;
    constexpr uint8_t SET_CONFIGTIME_COMMAND = 0x85;

    // Real-time clock
    constexpr uint8_t GET_RWC_COMMAND = 0x91;
    constexpr uint8_t RWC_RESPONSE = 0x90;
    constexpr uint8_t SET_RWC_COMMAND = 0x8F;

    // Device status
    constexpr uint8_t GET_STATUS_COMMAND = 0x72;
    constexpr uint8_t STATUS_RESPONSE = 0x71;

    // Firmware version
    constexpr uint8_t GET_FW_VERSION_COMMAND = 0x2E;
    constexpr uint8_t FW_VERSION_RESPONSE = 0x2F;

    // ExG (ADS1292R) register access
    constexpr uint8_t GET_EXG_REGS_COMMAND = 0x63;
    constexpr uint8_t EXG_REGS_RESPONSE = 0x62;
    constexpr uint8_t SET_EXG_REGS_COMMAND = 0x61;

    // Experiment ID
    constexpr uint8_t GET_EXPID_COMMAND = 0x7E;
    constexpr uint8_t EXPID_RESPONSE = 0x7D;
    constexpr uint8_t SET_EXPID_COMMAND = 0x7C;

    // Device name
    constexpr uint8_t GET_SHIMMERNAME_COMMAND = 0x7B;
    constexpr uint8_t SHIMMERNAME_RESPONSE = 0x7A;
    constexpr uint8_t SET_SHIMMERNAME_COMMAND = 0x79;

    // Miscellaneous
    constexpr uint8_t DUMMY_COMMAND = 0x96;
    constexpr uint8_t START_LOGGING_COMMAND = 0x92;
    constexpr uint8_t STOP_LOGGING_COMMAND = 0x93;
    constexpr uint8_t ENABLE_STATUS_ACK_COMMAND = 0xA3;

    // Calibration
    constexpr uint8_t GET_ALL_CALIBRATION_COMMAND = 0x2C;
    constexpr uint8_t ALL_CALIBRATION_RESPONSE = 0x2D;
    constexpr int ALL_CALIBRATION_LEN = 84; // 4 sensors, 21 bytes each
}


// Describes how a single channel's raw bytes should be decoded.
// Each channel in a Shimmer3 data packet has a fixed width, signedness,
// and byte order that we need to know at parse time.
struct ShimmerChannelDType
{
    int size; // width in bytes (1, 2, or 3)
    bool is_signed;
    bool little_endian;

    // Unpack raw bytes into a 32-bit signed integer, handling
    // byte order and sign extension.
    int32_t decode (const uint8_t *buf) const
    {
        uint32_t raw = 0;
        if (little_endian)
        {
            for (int i = size - 1; i >= 0; i--)
                raw = (raw << 8) | buf[i];
        }
        else
        {
            for (int i = 0; i < size; i++)
                raw = (raw << 8) | buf[i];
        }

        if (is_signed && (size < 4))
        {
            uint32_t sign_bit = 1u << (size * 8 - 1);
            if (raw & sign_bit)
                raw |= ~((1u << (size * 8)) - 1);
        }
        return static_cast<int32_t> (raw);
    }
};


// Identifies a data channel within a Shimmer3 data packet.
// The numeric values match the byte codes the device sends
// in its inquiry response to describe the active channel layout.
// TIMESTAMP is synthetic and it's always present as the first
// field in every packet but isn't listed in the inquiry.
enum class EChannelType : uint16_t
{
    ACCEL_LN_X = 0x00,
    ACCEL_LN_Y = 0x01,
    ACCEL_LN_Z = 0x02,
    VBATT = 0x03,
    ACCEL_WR_X = 0x04,
    ACCEL_WR_Y = 0x05,
    ACCEL_WR_Z = 0x06,
    MAG_REG_X = 0x07,
    MAG_REG_Y = 0x08,
    MAG_REG_Z = 0x09,
    GYRO_X = 0x0A,
    GYRO_Y = 0x0B,
    GYRO_Z = 0x0C,
    EXTERNAL_ADC_A0 = 0x0D,
    EXTERNAL_ADC_A1 = 0x0E,
    EXTERNAL_ADC_A2 = 0x0F,
    INTERNAL_ADC_A3 = 0x10,
    INTERNAL_ADC_A0 = 0x11,
    INTERNAL_ADC_A1 = 0x12,
    INTERNAL_ADC_A2 = 0x13,
    ACCEL_HG_X = 0x14,
    ACCEL_HG_Y = 0x15,
    ACCEL_HG_Z = 0x16,
    MAG_WR_X = 0x17,
    MAG_WR_Y = 0x18,
    MAG_WR_Z = 0x19,
    TEMPERATURE = 0x1A,
    PRESSURE = 0x1B,
    GSR_RAW = 0x1C,
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

    // Not a real on-wire channel — used internally to represent
    // the 3-byte timestamp that leads every data packet.
    TIMESTAMP = 0x100,
};


// Groups of related sensors that can be enabled or disabled together
// via the 3-byte sensor bitfield.
enum class ESensorGroup : int
{
    ACCEL_LN = 0,
    BATTERY,
    EXT_CH_A0,
    EXT_CH_A1,
    EXT_CH_A2,
    INT_CH_A0,
    INT_CH_A1,
    INT_CH_A2,
    STRAIN,
    INT_CH_A3,
    GSR,
    GYRO,
    ACCEL_WR,
    MAG_REG,
    ACCEL_HG,
    MAG_WR,
    TEMP,
    PRESSURE,
    EXG1_24BIT,
    EXG1_16BIT,
    EXG2_24BIT,
    EXG2_16BIT,
    SENSOR_GROUP_COUNT
};


// Shimmer3-specific numeric constants used for clock and timing math.
namespace Shimmer3Const
{
    // The Shimmer3's internal clock runs at 32768 Hz.
    constexpr double DEV_CLOCK_RATE = 32768.0;
    constexpr int ENABLED_SENSORS_LEN = 3;         // sensor bitfield is 3 bytes wide
    constexpr int TIMESTAMP_SIZE = 3;              // on-wire timestamp is 3 bytes
    constexpr uint32_t TIMESTAMP_MAX = (1u << 24); // 24-bit counter wraps here

    // The device stores sampling rate as a clock divider register.
    // Actual Hz = 32768 / divider.
    inline double dr2sr (uint16_t divider)
    {
        if (divider == 0)
            return 0.0;
        return DEV_CLOCK_RATE / static_cast<double> (divider);
    }

    inline uint16_t sr2dr (double hz)
    {
        if (hz <= 0.0)
            return 0;
        return static_cast<uint16_t> (DEV_CLOCK_RATE / hz);
    }

    inline double ticks2sec (uint64_t ticks)
    {
        return static_cast<double> (ticks) / DEV_CLOCK_RATE;
    }
}


// Maps a sensor group to its bit position in the 3-byte sensor
// enable/disable bitfield that the device accepts.
struct SensorBitEntry
{
    ESensorGroup group;
    int bit_position;
};

static const SensorBitEntry SENSOR_BIT_ASSIGNMENT[] = {
    {ESensorGroup::EXT_CH_A1, 0},
    {ESensorGroup::EXT_CH_A0, 1},
    {ESensorGroup::GSR, 2},
    {ESensorGroup::EXG2_24BIT, 3},
    {ESensorGroup::EXG1_24BIT, 4},
    {ESensorGroup::MAG_REG, 5},
    {ESensorGroup::GYRO, 6},
    {ESensorGroup::ACCEL_LN, 7},
    {ESensorGroup::INT_CH_A1, 8},
    {ESensorGroup::INT_CH_A0, 9},
    {ESensorGroup::INT_CH_A3, 10},
    {ESensorGroup::EXT_CH_A2, 11},
    {ESensorGroup::ACCEL_WR, 12},
    {ESensorGroup::BATTERY, 13},
    // bit 14 is unused
    {ESensorGroup::STRAIN, 15},
    // bit 16 is unused
    {ESensorGroup::TEMP, 17},
    {ESensorGroup::PRESSURE, 18},
    {ESensorGroup::EXG2_16BIT, 19},
    {ESensorGroup::EXG1_16BIT, 20},
    {ESensorGroup::MAG_WR, 21},
    {ESensorGroup::ACCEL_HG, 22},
    {ESensorGroup::INT_CH_A2, 23},
};
static constexpr int SENSOR_BIT_ASSIGNMENT_COUNT =
    sizeof (SENSOR_BIT_ASSIGNMENT) / sizeof (SENSOR_BIT_ASSIGNMENT[0]);


// Pairs each channel type with its wire format.
// Channels marked valid=false are defined in the protocol but not
// available on the Shimmer3 hardware (e.g. high-g accel).
struct ChannelDTypeEntry
{
    EChannelType channel;
    ShimmerChannelDType dtype;
    bool valid;
};

static const ChannelDTypeEntry CH_DTYPE_TABLE[] = {
    // Low-noise accelerometer: 2 bytes, signed, little-endian
    {EChannelType::ACCEL_LN_X, {2, true, true}, true},
    {EChannelType::ACCEL_LN_Y, {2, true, true}, true},
    {EChannelType::ACCEL_LN_Z, {2, true, true}, true},
    // Battery voltage
    {EChannelType::VBATT, {2, true, true}, true},
    // Wide-range accelerometer
    {EChannelType::ACCEL_WR_X, {2, true, true}, true},
    {EChannelType::ACCEL_WR_Y, {2, true, true}, true},
    {EChannelType::ACCEL_WR_Z, {2, true, true}, true},
    // Magnetometer
    {EChannelType::MAG_REG_X, {2, true, true}, true},
    {EChannelType::MAG_REG_Y, {2, true, true}, true},
    {EChannelType::MAG_REG_Z, {2, true, true}, true},
    // Gyroscope: 2 bytes, signed, big-endian
    {EChannelType::GYRO_X, {2, true, false}, true},
    {EChannelType::GYRO_Y, {2, true, false}, true},
    {EChannelType::GYRO_Z, {2, true, false}, true},
    // External ADC channels
    {EChannelType::EXTERNAL_ADC_A0, {2, false, true}, true},
    {EChannelType::EXTERNAL_ADC_A1, {2, false, true}, true},
    {EChannelType::EXTERNAL_ADC_A2, {2, false, true}, true},
    // Internal ADC channels
    {EChannelType::INTERNAL_ADC_A3, {2, false, true}, true},
    {EChannelType::INTERNAL_ADC_A0, {2, false, true}, true},
    {EChannelType::INTERNAL_ADC_A1, {2, false, true}, true},
    {EChannelType::INTERNAL_ADC_A2, {2, false, true}, true},
    // High-g accel
    {EChannelType::ACCEL_HG_X, {0, false, false}, false},
    {EChannelType::ACCEL_HG_Y, {0, false, false}, false},
    {EChannelType::ACCEL_HG_Z, {0, false, false}, false},
    // Wide-range mag
    {EChannelType::MAG_WR_X, {0, false, false}, false},
    {EChannelType::MAG_WR_Y, {0, false, false}, false},
    {EChannelType::MAG_WR_Z, {0, false, false}, false},
    // Temperature and pressure
    {EChannelType::TEMPERATURE, {2, false, false}, true},
    {EChannelType::PRESSURE, {3, false, false}, true},
    // GSR (galvanic skin response)
    {EChannelType::GSR_RAW, {2, false, true}, true},
    // ExG chip 1 (ADS1292R): status + two data channels
    {EChannelType::EXG1_STATUS, {1, false, true}, true},
    {EChannelType::EXG1_CH1_24BIT, {3, true, false}, true},
    {EChannelType::EXG1_CH2_24BIT, {3, true, false}, true},
    // ExG chip 2
    {EChannelType::EXG2_STATUS, {1, false, true}, true},
    {EChannelType::EXG2_CH1_24BIT, {3, true, false}, true},
    {EChannelType::EXG2_CH2_24BIT, {3, true, false}, true},
    // 16-bit ExG variants
    {EChannelType::EXG1_CH1_16BIT, {2, true, false}, true},
    {EChannelType::EXG1_CH2_16BIT, {2, true, false}, true},
    {EChannelType::EXG2_CH1_16BIT, {2, true, false}, true},
    {EChannelType::EXG2_CH2_16BIT, {2, true, false}, true},
    // Strain gauge
    {EChannelType::STRAIN_HIGH, {2, false, true}, true},
    {EChannelType::STRAIN_LOW, {2, false, true}, true},
    // Timestamp (3 bytes, unsigned, little-endian)
    {EChannelType::TIMESTAMP, {3, false, true}, true},
};
static constexpr int CH_DTYPE_TABLE_COUNT = sizeof (CH_DTYPE_TABLE) / sizeof (CH_DTYPE_TABLE[0]);


// Per-sensor calibration parameters.
// The Shimmer3 stores offset, sensitivity, and a 3×3 alignment
// matrix for each of its four calibrated sensor groups.
struct ShimmerCalibrationSensor
{
    int16_t offset_bias[3]; // 3-axis offset, big-endian on wire
    int16_t sensitivity[3]; // 3-axis sensitivity, big-endian on wire
    int8_t alignment[9];    // 3×3 alignment matrix, row-major
};

// Holds calibration data for all four sensor groups:
// low-noise accel, gyro, magnetometer, wide-range accel.
// The device returns all 84 bytes in one response.
struct ShimmerAllCalibration
{
    ShimmerCalibrationSensor sensors[4]; // 0=ACCEL_LN, 1=GYRO, 2=MAG, 3=ACCEL_WR
    bool valid;

    ShimmerAllCalibration () : valid (false)
    {
        memset (sensors, 0, sizeof (sensors));
    }

    bool parse (const uint8_t *data, int len)
    {
        if (len < 84)
            return false;

        for (int s = 0; s < 4; s++)
        {
            const uint8_t *p = data + s * 21;

            for (int i = 0; i < 3; i++)
            {
                sensors[s].offset_bias[i] = static_cast<int16_t> ((p[i * 2] << 8) | p[i * 2 + 1]);
            }
            for (int i = 0; i < 3; i++)
            {
                sensors[s].sensitivity[i] =
                    static_cast<int16_t> ((p[6 + i * 2] << 8) | p[6 + i * 2 + 1]);
            }
            for (int i = 0; i < 9; i++)
            {
                sensors[s].alignment[i] = static_cast<int8_t> (p[12 + i]);
            }
        }
        valid = true;
        return true;
    }
};
