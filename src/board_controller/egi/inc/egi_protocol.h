#pragma once

#include <array>
#include <cctype>
#include <cstdint>
#include <string>

namespace egi
{
    inline uint64_t le64 (const uint8_t *data)
    {
        uint64_t result = 0;
        for (size_t i = 0; i < 8; ++i)
            result |= uint64_t (data[i]) << (8 * i);
        return result;
    }

    inline uint64_t be64 (const uint8_t *data)
    {
        uint64_t result = 0;
        for (size_t i = 0; i < 8; ++i)
            result = (result << 8) | data[i];
        return result;
    }

    inline bool header (const uint8_t *data, uint64_t amplifier, size_t &length)
    {
        uint64_t size = be64 (data + 8);
        if ((be64 (data) != amplifier) || (size == 0) || (size % 1264 != 0) || (size > 1264 * 4096))
            return false;
        length = size_t (size);
        return true;
    }

    inline std::string field (const std::string &response, const std::string &key)
    {
        std::string prefix = "(" + key;
        size_t offset = response.find (prefix);
        if (offset == std::string::npos)
            return "";
        offset += prefix.size ();
        if ((offset == response.size ()) ||
            !std::isspace (static_cast<unsigned char> (response[offset])))
            return "";
        while ((offset < response.size ()) &&
            std::isspace (static_cast<unsigned char> (response[offset])))
            ++offset;
        size_t start = offset;
        while ((offset < response.size ()) && (response[offset] != ')') &&
            !std::isspace (static_cast<unsigned char> (response[offset])))
            ++offset;
        if ((offset == response.size ()) || (offset == start))
            return "";
        return response.substr (start, offset - start);
    }

    struct Sample
    {
        uint64_t counter;
        std::array<double, 264> values;
    };

    inline bool decode (const uint8_t *data, size_t size, Sample &sample)
    {
        if (size != 1264)
            return false;
        // Fixed 256-channel profile; test connector is useful for vendor signal tests.
        if ((data[41] != 2) && (data[41] != 6) && (data[41] != 10) && (data[41] != 14))
            return false;
        sample.values.fill (0);
        sample.counter = le64 (data + 25);
        sample.values[0] = double (sample.counter);
        for (size_t channel = 0; channel < 256; ++channel)
        {
            const uint8_t *p = data + 80 + 4 * channel;
            uint32_t word = uint32_t (p[0]) | (uint32_t (p[1]) << 8) | (uint32_t (p[2]) << 16) |
                (uint32_t (p[3]) << 24);
            int64_t value = (word & 0x80000000) ? int64_t (word) - 0x100000000LL : int64_t (word);
            // NA400 factor from the EGI AmpServer LSL reference; NA500 uses a different gain.
            sample.values[channel + 1] = double (value) * 0.00015522042;
        }
        uint16_t digital = uint16_t (data[0]) | (uint16_t (data[1]) << 8);
        sample.values[257] = digital;
        sample.values[258] = uint16_t (~digital);
        sample.values[259] = data[2];
        sample.values[260] = data[41];
        sample.values[261] = double (le64 (data + 33));
        return true;
    }
}
