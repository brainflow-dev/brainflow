#include "rda_protocol.h"
#include <gtest/gtest.h>

namespace
{
    void add32 (std::vector<uint8_t> &data, uint32_t value)
    {
        for (int i = 0; i < 4; ++i)
            data.push_back (uint8_t (value >> (8 * i)));
    }
    void add64 (std::vector<uint8_t> &data, double value)
    {
        uint64_t bits;
        std::memcpy (&bits, &value, sizeof (bits));
        add32 (data, uint32_t (bits));
        add32 (data, uint32_t (bits >> 32));
    }
    std::vector<uint8_t> start ()
    {
        std::vector<uint8_t> data;
        add32 (data, 32);
        add64 (data, 1000.0);
        for (int i = 0; i < 32; ++i)
            add64 (data, 0.5);
        for (int i = 0; i < 32; ++i)
            data.insert (data.end (), {'E', uint8_t ('A' + i), 0});
        return data;
    }
    std::vector<uint8_t> block ()
    {
        std::vector<uint8_t> data;
        add32 (data, 42);
        add32 (data, 2);
        add32 (data, 2);
        for (int i = 0; i < 64; ++i)
            add32 (data, 0xc0600000); // -3.5, IEEE-754 binary32
        for (int i = 0; i < 2; ++i)
        {
            add32 (data, 30);
            add32 (data, 1);
            add32 (data, 1);
            add32 (data, 0xffffffff);
            const std::string label = "Stimulus";
            data.insert (data.end (), label.begin (), label.end ());
            data.insert (data.end (), {0, 'S', ' ', '1', '2', 0});
        }
        return data;
    }
}

TEST (BrainVisionRDA, ChecksUuidAndBoundsBeforeAllocation)
{
    std::vector<uint8_t> data = {0x8e, 0x45, 0x58, 0x43, 0x96, 0xc9, 0x86, 0x4c, 0xaf, 0x4a, 0x98,
        0xbb, 0xf6, 0xc9, 0x14, 0x50};
    add32 (data, 24);
    add32 (data, 3);
    uint32_t size, type;
    ASSERT_TRUE (rda::header (data.data (), size, type));
    EXPECT_EQ (size, 24u);
    EXPECT_EQ (type, 3u);
    data[0] ^= 1;
    EXPECT_FALSE (rda::header (data.data (), size, type));
    data[0] ^= 1;
    data[16] = 23;
    EXPECT_FALSE (rda::header (data.data (), size, type));
    data[19] = 0xff;
    EXPECT_FALSE (rda::header (data.data (), size, type));
}

TEST (BrainVisionRDA, ScalesDataAndPreservesCoincidentMarkers)
{
    rda::Decoder decoder;
    rda::Block result;
    EXPECT_FALSE (decoder.data32 (block (), result));
    ASSERT_TRUE (decoder.start (start ()));
    ASSERT_EQ (decoder.names.size (), 32u);
    EXPECT_EQ (decoder.names[0], "EA");
    ASSERT_TRUE (decoder.data32 (block (), result));
    ASSERT_EQ (result.samples.size (), 2u);
    EXPECT_EQ (result.number, 42u);
    EXPECT_DOUBLE_EQ (result.samples[0][1], -1.75);
    EXPECT_EQ (result.samples[0][33], 0);
    EXPECT_EQ (result.samples[1][33], 2);
    EXPECT_EQ (result.samples[1][34], 1);
    ASSERT_EQ (result.markers.size (), 2u);
    EXPECT_EQ (result.markers[0].position, 1u);
    EXPECT_EQ (result.markers[0].channel, -1);
    EXPECT_EQ (result.markers[0].description, "S 12");
}

TEST (BrainVisionRDA, RejectsOtherProfilesAndInvalidMetadata)
{
    rda::Decoder decoder;
    auto data = start ();
    data[0] = 31;
    EXPECT_FALSE (decoder.start (data));
    data = start ();
    std::vector<uint8_t> rate;
    add64 (rate, 2000.0);
    std::copy (rate.begin (), rate.end (), data.begin () + 4);
    EXPECT_FALSE (decoder.start (data));
    data = start ();
    std::fill (data.begin () + 12, data.begin () + 20, 0);
    EXPECT_FALSE (decoder.start (data));
    data = start ();
    data.pop_back ();
    EXPECT_FALSE (decoder.start (data));
}

TEST (BrainVisionRDA, RejectsTruncatedAndMaliciousMarkersAtomically)
{
    rda::Decoder decoder;
    ASSERT_TRUE (decoder.start (start ()));
    rda::Block result;
    result.number = 99;
    auto data = block ();
    for (size_t size = 0; size < data.size (); ++size)
    {
        EXPECT_FALSE (
            decoder.data32 (std::vector<uint8_t> (data.begin (), data.begin () + size), result));
        EXPECT_EQ (result.number, 99u);
    }
    data[268] = 60; // First marker consumes the second: must reject before reading past the buffer.
    EXPECT_FALSE (decoder.data32 (data, result));
    data = block ();
    data[272] = 2; // position == sample count
    EXPECT_FALSE (decoder.data32 (data, result));
    data = block ();
    data[280] = 32;
    data[281] = data[282] = data[283] = 0;
    EXPECT_FALSE (decoder.data32 (data, result));
    data = block ();
    data.push_back (0);
    EXPECT_FALSE (decoder.data32 (data, result));
}
