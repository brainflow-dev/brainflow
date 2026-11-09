#include "thinkgear_parser.h"
#include <cmath>
#include <gtest/gtest.h>

namespace
{
    std::vector<uint8_t> frame (std::vector<uint8_t> payload)
    {
        uint8_t sum = 0;
        for (auto byte : payload)
            sum = uint8_t (sum + byte);
        std::vector<uint8_t> packet = {0xaa, 0xaa, uint8_t (payload.size ())};
        packet.insert (packet.end (), payload.begin (), payload.end ());
        packet.push_back (uint8_t (~sum));
        return packet;
    }
    std::vector<thinkgear::Sample> feed (thinkgear::Parser &parser, std::vector<uint8_t> bytes)
    {
        std::vector<thinkgear::Sample> result;
        for (auto byte : bytes)
        {
            auto samples = parser.feed (byte);
            result.insert (result.end (), samples.begin (), samples.end ());
        }
        return result;
    }
}

TEST (ThinkGear, PublishedMetadataPacketAndSignedRawSamples)
{
    thinkgear::Parser parser;
    EXPECT_TRUE (feed (parser, {0xaa, 0xaa, 8, 2, 0x20, 1, 0x7e, 4, 0x12, 5, 0x60, 0xe3}).empty ());
    auto samples = feed (parser, {0xaa, 0xaa, 4, 0x80, 2, 0xff, 0xff, 0x7f});
    ASSERT_EQ (samples.size (), 1u);
    EXPECT_DOUBLE_EQ (samples[0].eeg, -0.2197265625);
    EXPECT_EQ (samples[0].auxiliary[0], 32);
    EXPECT_EQ (samples[0].auxiliary[1], 18);
    EXPECT_EQ (samples[0].auxiliary[2], 96);
    EXPECT_TRUE (std::isnan (samples[0].auxiliary[3]));
    samples = feed (parser, frame ({0x80, 2, 0x80, 0, 0x80, 2, 0x7f, 0xff}));
    ASSERT_EQ (samples.size (), 2u);
    EXPECT_DOUBLE_EQ (samples[0].eeg, -7200.0);
    EXPECT_DOUBLE_EQ (samples[1].eeg, 32767 * 0.2197265625);
}

TEST (ThinkGear, RecoversAfterNoiseChecksumErrorAndRepeatedSync)
{
    thinkgear::Parser parser;
    auto bad = frame ({0x80, 2, 0, 1});
    bad.back () ^= 1;
    EXPECT_TRUE (feed (parser, bad).empty ());
    EXPECT_TRUE (feed (parser, {0, 0xfe, 0xaa, 0xaa, 0xaa}).empty ());
    auto good = frame ({0x80, 2, 0, 2});
    for (size_t i = 0; i + 1 < good.size (); ++i)
        EXPECT_TRUE (parser.feed (good[i]).empty ());
    auto samples = parser.feed (good.back ());
    ASSERT_EQ (samples.size (), 1u);
    EXPECT_DOUBLE_EQ (samples[0].eeg, 0.439453125);
}

TEST (ThinkGear, RejectsWholeMalformedPayloadAndSkipsExtendedRows)
{
    thinkgear::Parser parser;
    EXPECT_TRUE (feed (parser, frame ({2, 200, 0x80, 2, 0, 1, 0x83, 24, 0})).empty ());
    auto samples = feed (parser, frame ({0x55, 2, 99, 0x90, 3, 1, 2, 3, 0x80, 2, 0, 1}));
    ASSERT_EQ (samples.size (), 1u);
    EXPECT_TRUE (std::isnan (samples[0].auxiliary[0]));
    EXPECT_TRUE (feed (parser, frame ({0x80, 1, 0})).empty ());
    EXPECT_TRUE (feed (parser, frame ({0x55})).empty ());
    EXPECT_TRUE (feed (parser, frame ({0x90})).empty ());
}

TEST (ThinkGear, AsicPowersAndReset)
{
    thinkgear::Parser parser;
    std::vector<uint8_t> payload = {0x83, 24};
    for (int i = 0; i < 8; ++i)
        payload.insert (payload.end (), {1, 2, uint8_t (i)});
    payload.insert (payload.end (), {0x80, 2, 0, 0});
    auto samples = feed (parser, frame (payload));
    ASSERT_EQ (samples.size (), 1u);
    for (int i = 0; i < 8; ++i)
        EXPECT_EQ (samples[0].auxiliary[i + 3], 66048 + i);
    parser.reset ();
    samples = feed (parser, frame ({0x80, 2, 0, 0}));
    ASSERT_EQ (samples.size (), 1u);
    EXPECT_TRUE (std::isnan (samples[0].auxiliary[3]));
}
