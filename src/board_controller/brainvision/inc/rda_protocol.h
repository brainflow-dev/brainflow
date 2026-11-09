#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace rda
{
    inline uint32_t u32 (const uint8_t *data)
    {
        return uint32_t (data[0]) | (uint32_t (data[1]) << 8) | (uint32_t (data[2]) << 16) |
            (uint32_t (data[3]) << 24);
    }

    inline double f64 (const uint8_t *data)
    {
        uint64_t bits = uint64_t (u32 (data)) | (uint64_t (u32 (data + 4)) << 32);
        double value;
        std::memcpy (&value, &bits, sizeof (value));
        return value;
    }

    inline float f32 (const uint8_t *data)
    {
        uint32_t bits = u32 (data);
        float value;
        std::memcpy (&value, &bits, sizeof (value));
        return value;
    }

    inline bool header (const uint8_t *data, uint32_t &size, uint32_t &type)
    {
        const uint8_t uuid[] = {0x8e, 0x45, 0x58, 0x43, 0x96, 0xc9, 0x86, 0x4c, 0xaf, 0x4a, 0x98,
            0xbb, 0xf6, 0xc9, 0x14, 0x50};
        size = u32 (data + 16);
        type = u32 (data + 20);
        return std::memcmp (uuid, data, sizeof (uuid)) == 0 && size >= 24 &&
            size <= 8 * 1024 * 1024;
    }

    inline bool text (
        const std::vector<uint8_t> &data, size_t &offset, size_t end, std::string &value)
    {
        size_t start = offset;
        while ((offset < end) && data[offset] && (offset - start <= 512))
            ++offset;
        if ((offset == end) || (offset - start > 512))
            return false;
        value.assign (data.begin () + start, data.begin () + offset);
        ++offset;
        return true;
    }

    struct Marker
    {
        uint32_t block;
        uint32_t position;
        uint32_t points;
        int64_t channel;
        std::string type;
        std::string description;
    };

    struct Block
    {
        uint32_t number;
        std::vector<std::array<double, 37>> samples;
        std::vector<Marker> markers;
    };

    class Decoder
    {
        bool configured = false;
        std::array<double, 32> resolutions;

    public:
        std::vector<std::string> names;

        bool start (const std::vector<uint8_t> &data)
        {
            configured = false;
            names.clear ();
            if ((data.size () < 268) || (u32 (data.data ()) != 32))
                return false;
            double interval = f64 (data.data () + 4);
            if (!std::isfinite (interval) || (std::abs (interval - 1000.0) > 1e-6))
                return false;
            for (size_t i = 0; i < 32; ++i)
            {
                resolutions[i] = f64 (data.data () + 12 + 8 * i);
                if (!std::isfinite (resolutions[i]) || (resolutions[i] <= 0))
                    return false;
            }
            size_t offset = 268;
            for (size_t i = 0; i < 32; ++i)
            {
                std::string name;
                if (!text (data, offset, data.size (), name))
                    return false;
                names.push_back (name);
            }
            configured = offset == data.size ();
            return configured;
        }

        bool data32 (const std::vector<uint8_t> &data, Block &result) const
        {
            if (!configured || (data.size () < 12))
                return false;
            uint32_t points = u32 (data.data () + 4);
            uint32_t marker_count = u32 (data.data () + 8);
            if ((points == 0) || (points > (data.size () - 12) / 128))
                return false;
            Block pending;
            pending.number = u32 (data.data ());
            pending.samples.resize (points);
            size_t offset = 12;
            for (size_t sample = 0; sample < points; ++sample)
            {
                auto &row = pending.samples[sample];
                row.fill (0);
                row[0] = pending.number;
                row[34] = double (sample);
                for (size_t channel = 0; channel < 32; ++channel, offset += 4)
                    row[channel + 1] = double (f32 (data.data () + offset)) * resolutions[channel];
            }
            // At least 16 fixed bytes and two terminating NULs per marker.
            if (marker_count > (data.size () - offset) / 18)
                return false;
            for (uint32_t i = 0; i < marker_count; ++i)
            {
                if (data.size () - offset < 18)
                    return false;
                uint32_t size = u32 (data.data () + offset);
                if ((size < 18) || (size > data.size () - offset))
                    return false;
                size_t end = offset + size;
                Marker marker;
                marker.block = pending.number;
                marker.position = u32 (data.data () + offset + 4);
                marker.points = u32 (data.data () + offset + 8);
                uint32_t channel = u32 (data.data () + offset + 12);
                marker.channel = (channel == 0xffffffff) ? -1 : int64_t (channel);
                if ((marker.position >= points) ||
                    ((marker.channel < -1) || (marker.channel >= 32)))
                    return false;
                offset += 16;
                if (!text (data, offset, end, marker.type) ||
                    !text (data, offset, end, marker.description) || (offset != end))
                    return false;
                pending.samples[marker.position][33] += 1;
                pending.markers.push_back (marker);
            }
            if (offset != data.size ())
                return false;
            result = std::move (pending);
            return true;
        }
    };
}
