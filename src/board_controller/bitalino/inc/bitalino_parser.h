#pragma once

#include <stddef.h>

#include <array>
#include <cstdint>
#include <vector>

// Six-channel BITalino live/simulated frame: 52 analog bits, four digital bits,
// CRC4 and a four-bit sequence number. Analog values are uncalibrated ADC counts.
namespace bitalino
{
    using Sample = std::array<double, 11>;

    inline bool valid (const uint8_t *data)
    {
        unsigned remainder = 0;
        for (size_t byte = 0; byte < 8; ++byte)
        {
            unsigned value = (byte == 7) ? (data[byte] & 0xf0) : data[byte];
            for (int bit = 7; bit >= 0; --bit)
            {
                remainder = (remainder << 1) | ((value >> bit) & 1);
                if (remainder & 0x10)
                    remainder ^= 0x13; // x^4 + x + 1
            }
        }
        return remainder == (data[7] & 15);
    }

    inline Sample decode (const uint8_t *data)
    {
        uint64_t word = 0;
        for (size_t i = 0; i < 8; ++i)
            word |= uint64_t (data[i]) << (8 * i);
        Sample sample = {{double (word >> 60)}};
        const unsigned shifts[] = {42, 32, 22, 12, 6, 0};
        for (size_t i = 0; i < 6; ++i)
            sample[i + 1] = double ((word >> shifts[i]) & ((i < 4) ? 1023 : 63));
        for (size_t i = 0; i < 4; ++i)
            sample[i + 7] = double ((word >> (55 - i)) & 1);
        return sample;
    }

    class Parser
    {
        std::vector<uint8_t> buffer;
        bool synchronized = false;

    public:
        std::vector<Sample> feed (uint8_t byte)
        {
            std::vector<Sample> samples;
            buffer.push_back (byte);
            while (buffer.size () >= (synchronized ? 8u : 16u))
            {
                bool ok = valid (buffer.data ());
                if (!synchronized)
                    ok = ok && valid (buffer.data () + 8) &&
                        (((buffer[7] >> 4) + 1) % 16 == (buffer[15] >> 4));
                if (!ok)
                {
                    synchronized = false;
                    buffer.erase (buffer.begin ());
                    continue;
                }
                synchronized = true;
                samples.push_back (decode (buffer.data ()));
                buffer.erase (buffer.begin (), buffer.begin () + 8);
            }
            return samples;
        }
    };
}
