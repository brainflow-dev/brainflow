#pragma once

#include <stddef.h>

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

// ThinkGear binary protocol, TGAT/TGAM 16-bit EEG mode (512 Hz).
// https://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol
namespace thinkgear
{
    struct Sample
    {
        double eeg;
        std::array<double, 11> auxiliary; // quality, attention, meditation, eight ASIC powers
    };

    class Parser
    {
        std::vector<uint8_t> buffer;
        std::array<double, 11> auxiliary;

        bool payload (const uint8_t *data, size_t size, std::vector<Sample> &samples)
        {
            auto next = auxiliary;
            std::vector<Sample> pending;
            size_t offset = 0;
            while (offset < size)
            {
                bool extended = false;
                while ((offset < size) && (data[offset] == 0x55))
                {
                    extended = true;
                    ++offset;
                }
                if (offset == size)
                    return false;
                uint8_t code = data[offset++];
                size_t length = 1;
                if (code >= 0x80)
                {
                    if (offset == size)
                        return false;
                    length = data[offset++];
                }
                if (length > size - offset)
                    return false;
                if (!extended)
                {
                    if ((code == 0x80) && (length != 2))
                        return false;
                    if ((code == 0x83) && (length != 24))
                        return false;
                    if (code == 0x80)
                    {
                        int raw = (int (data[offset]) << 8) | data[offset + 1];
                        if (raw >= 32768)
                            raw -= 65536;
                        // NeuroSky's TGAT conversion: raw * (1.8 / 4096) / 2000 V.
                        pending.push_back ({raw * (1.8 / 4096.0 / 2000.0 * 1e6), next});
                    }
                    else if (code == 0x02)
                        next[0] = data[offset];
                    else if (code == 0x04)
                        next[1] = data[offset];
                    else if (code == 0x05)
                        next[2] = data[offset];
                    else if (code == 0x83)
                    {
                        for (size_t i = 0; i < 8; ++i)
                            next[i + 3] = (uint32_t (data[offset + i * 3]) << 16) |
                                (uint32_t (data[offset + i * 3 + 1]) << 8) |
                                data[offset + i * 3 + 2];
                    }
                }
                offset += length;
            }
            auxiliary = next;
            samples.insert (samples.end (), pending.begin (), pending.end ());
            return true;
        }

    public:
        Parser ()
        {
            reset ();
        }

        void reset ()
        {
            buffer.clear ();
            auxiliary.fill (std::numeric_limits<double>::quiet_NaN ());
        }

        std::vector<Sample> feed (uint8_t byte)
        {
            std::vector<Sample> samples;
            buffer.push_back (byte);
            while (buffer.size () >= 3)
            {
                if ((buffer[0] != 0xaa) || (buffer[1] != 0xaa) || (buffer[2] > 169))
                {
                    buffer.erase (buffer.begin ());
                    continue;
                }
                size_t length = buffer[2];
                if (buffer.size () < length + 4)
                    break;
                uint8_t sum = 0;
                for (size_t i = 3; i < length + 4; ++i)
                    sum = uint8_t (sum + buffer[i]);
                if (sum != 0xff)
                {
                    buffer.erase (buffer.begin ());
                    continue;
                }
                payload (buffer.data () + 3, length, samples);
                buffer.erase (buffer.begin (), buffer.begin () + length + 4);
            }
            return samples;
        }
    };
}
