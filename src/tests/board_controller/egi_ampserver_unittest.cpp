#include "egi_protocol.h"
#include <gtest/gtest.h>
#include <vector>

TEST (EGIAmpServer, ValidatesBigEndianBlockHeader)
{
    uint8_t data[16] = {0};
    data[7] = 2;
    data[14] = 4;
    data[15] = 0xf0; // 1264
    size_t length = 0;
    ASSERT_TRUE (egi::header (data, 2, length));
    EXPECT_EQ (length, 1264u);
    EXPECT_FALSE (egi::header (data, 0, length));
    data[15] = 0xf1;
    EXPECT_FALSE (egi::header (data, 2, length));
    data[8] = 1;
    EXPECT_FALSE (egi::header (data, 2, length));
}

TEST (EGIAmpServer, DecodesUnalignedLittleEndianNa400Frame)
{
    std::vector<uint8_t> data (1264, 0);
    data[0] = 0xfe;
    data[1] = 0xff;
    data[2] = 7;
    data[25] = 0x78;
    data[26] = 0x56;
    data[27] = 0x34;
    data[28] = 0x12;
    data[29] = 1;
    data[33] = 42;
    data[41] = 6;
    // Channel 1 = -1000000; channel 256 = 1000000.
    const uint8_t negative[] = {0xc0, 0xbd, 0xf0, 0xff};
    const uint8_t positive[] = {0x40, 0x42, 0x0f, 0};
    std::copy (negative, negative + 4, data.begin () + 80);
    std::copy (positive, positive + 4, data.begin () + 1100);
    egi::Sample sample;
    ASSERT_TRUE (egi::decode (data.data (), data.size (), sample));
    EXPECT_EQ (sample.counter, 0x112345678ULL);
    EXPECT_DOUBLE_EQ (sample.values[1], -155.22042);
    EXPECT_DOUBLE_EQ (sample.values[256], 155.22042);
    EXPECT_EQ (sample.values[257], 65534);
    EXPECT_EQ (sample.values[258], 1);
    EXPECT_EQ (sample.values[259], 7);
    EXPECT_EQ (sample.values[260], 6);
    EXPECT_EQ (sample.values[261], 42);
    EXPECT_FALSE (egi::decode (data.data (), 1263, sample));
    data[41] = 5; // 128-channel net is outside this fixed profile.
    EXPECT_FALSE (egi::decode (data.data (), data.size (), sample));
}

TEST (EGIAmpServer, MatchesCompleteMetadataTokens)
{
    EXPECT_EQ (egi::field ("(status complete) (amp_type\t NA400)", "amp_type"), "NA400");
    EXPECT_EQ (egi::field ("(amp_type_other NA400)", "amp_type"), "");
    EXPECT_EQ (egi::field ("(amp_type NA400", "amp_type"), "");
    EXPECT_EQ (egi::field ("(amp_type )", "amp_type"), "");
    EXPECT_EQ (egi::field ("(status error)", "status"), "error");
}
