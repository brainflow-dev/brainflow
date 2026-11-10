#include "bitalino_parser.h"
#include <gtest/gtest.h>

namespace
{
    // Golden vectors generated with the vendor API's CRC lookup table.
    const std::vector<uint8_t> zero = {209, 175, 106, 85, 0, 254, 175, 15};
    const std::vector<uint8_t> one = {209, 175, 106, 85, 0, 254, 175, 28};
    const std::vector<uint8_t> fifteen = {209, 175, 106, 85, 0, 254, 175, 253};
    std::vector<bitalino::Sample> feed (bitalino::Parser &parser, const std::vector<uint8_t> &data)
    {
        std::vector<bitalino::Sample> result;
        for (auto byte : data)
        {
            auto samples = parser.feed (byte);
            result.insert (result.end (), samples.begin (), samples.end ());
        }
        return result;
    }
}

TEST (BITalino, SixChannelsDigitalAndCrc)
{
    ASSERT_TRUE (bitalino::valid (zero.data ()));
    auto sample = bitalino::decode (zero.data ());
    bitalino::Sample expected = {{0, 1023, 512, 341, 682, 63, 17, 1, 0, 1, 0}};
    EXPECT_EQ (sample, expected);
    for (size_t byte = 0; byte < zero.size (); ++byte)
        for (int bit = 0; bit < 8; ++bit)
        {
            auto corrupt = zero;
            corrupt[byte] ^= uint8_t (1 << bit);
            EXPECT_FALSE (bitalino::valid (corrupt.data ()));
        }
}

TEST (BITalino, SynchronizesAcrossSequenceWrapAndFragmentation)
{
    bitalino::Parser parser;
    EXPECT_TRUE (feed (parser, fifteen).empty ());
    auto samples = feed (parser, zero);
    ASSERT_EQ (samples.size (), 2u);
    EXPECT_EQ (samples[0][0], 15);
    EXPECT_EQ (samples[1][0], 0);
    samples = feed (parser, one);
    ASSERT_EQ (samples.size (), 1u);
    EXPECT_EQ (samples[0][0], 1);
}

TEST (BITalino, RecoversFromLostBytesWithoutPublishingCorruptFrame)
{
    bitalino::Parser parser;
    auto corrupt = zero;
    corrupt[0] ^= 1;
    EXPECT_TRUE (feed (parser, corrupt).empty ());
    EXPECT_TRUE (feed (parser, zero).empty ());
    auto samples = feed (parser, one);
    ASSERT_EQ (samples.size (), 2u);
    EXPECT_EQ (samples[0], bitalino::decode (zero.data ()));
    EXPECT_EQ (samples[1], bitalino::decode (one.data ()));
}
