#pragma once

#include <stddef.h>

#include <cstdint>
#include <limits>
#include <vector>

namespace polar
{
    inline uint16_t u16 (const uint8_t *data)
    {
        return uint16_t (data[0]) | (uint16_t (data[1]) << 8);
    }

    struct Ecg
    {
        uint64_t timestamp;
        std::vector<int32_t> samples;
    };

    inline bool ecg (const uint8_t *data, size_t size, Ecg &result)
    {
        if ((size < 13) || (data[0] != 0) || (data[9] != 0) || ((size - 10) % 3 != 0))
            return false;
        Ecg pending;
        pending.timestamp = 0;
        for (size_t i = 0; i < 8; ++i)
            pending.timestamp |= uint64_t (data[i + 1]) << (8 * i);
        for (size_t i = 10; i < size; i += 3)
        {
            int32_t value =
                int32_t (data[i]) | (int32_t (data[i + 1]) << 8) | (int32_t (data[i + 2]) << 16);
            if (value & 0x800000)
                value -= 0x1000000;
            pending.samples.push_back (value); // Already microvolts, no ADC gain conversion.
        }
        result = pending;
        return true;
    }

    inline bool supports_ecg_profile (const std::vector<uint8_t> &data)
    {
        bool rate = false, resolution = false;
        size_t offset = 0;
        // PMD setting widths, excluding security and unknown/new setting kinds.
        const size_t widths[] = {2, 2, 2, 4, 1, 4};
        while (offset < data.size ())
        {
            if (data.size () - offset < 2)
                return false;
            uint8_t type = data[offset++];
            size_t count = data[offset++];
            if ((type >= 6) || (count > (data.size () - offset) / widths[type]))
                return false;
            for (size_t i = 0; i < count; ++i, offset += widths[type])
            {
                if ((type == 0) && (u16 (data.data () + offset) == 130))
                    rate = true;
                if ((type == 1) && (u16 (data.data () + offset) == 14))
                    resolution = true;
            }
        }
        return rate && resolution;
    }

    struct HeartRate
    {
        double bpm;
        double contact;
        std::vector<double> rr_ms;
    };

    inline bool heart_rate (const uint8_t *data, size_t size, HeartRate &result)
    {
        if (size < 2)
            return false;
        uint8_t flags = data[0];
        size_t offset = (flags & 1) ? 3 : 2;
        if (size < offset)
            return false;
        HeartRate pending;
        pending.bpm = (flags & 1) ? u16 (data + 1) : data[1];
        pending.contact =
            (flags & 4) ? ((flags & 2) ? 1.0 : 0.0) : std::numeric_limits<double>::quiet_NaN ();
        if (flags & 8)
            offset += 2; // Energy expended is not exposed by this profile.
        if (offset > size)
            return false;
        if (flags & 16)
        {
            if ((size == offset) || ((size - offset) % 2 != 0))
                return false;
            for (; offset < size; offset += 2)
                pending.rr_ms.push_back (u16 (data + offset) * (1000.0 / 1024.0));
        }
        if (offset != size)
            return false;
        result = pending;
        return true;
    }
}
