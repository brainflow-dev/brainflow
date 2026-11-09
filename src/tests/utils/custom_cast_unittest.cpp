#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>

#include "custom_cast.h"

using namespace testing;


TEST (CustomCastTest, Cast24BitToInt32_BoundaryAndSignedValues_ReturnsExpectedTwosComplement)
{
    const unsigned char zero[3] = {0x00, 0x00, 0x00};
    const unsigned char pos_one[3] = {0x00, 0x00, 0x01};
    const unsigned char pos_max[3] = {0x7F, 0xFF, 0xFF};
    const unsigned char neg_one[3] = {0xFF, 0xFF, 0xFF};
    const unsigned char neg_two[3] = {0xFF, 0xFF, 0xFE};
    const unsigned char neg_min[3] = {0x80, 0x00, 0x00};

    EXPECT_EQ (cast_24bit_to_int32 (zero), 0);
    EXPECT_EQ (cast_24bit_to_int32 (pos_one), 1);
    EXPECT_EQ (cast_24bit_to_int32 (pos_max), 8388607);
    EXPECT_EQ (cast_24bit_to_int32 (neg_one), -1);
    EXPECT_EQ (cast_24bit_to_int32 (neg_two), -2);
    EXPECT_EQ (cast_24bit_to_int32 (neg_min), -8388608);
}

TEST (CustomCastTest, Cast24BitToInt32SwapOrder_LittleEndianInput_ReturnsExpectedTwosComplement)
{
    const unsigned char pos_max_le[3] = {0xFF, 0xFF, 0x7F};
    const unsigned char neg_one_le[3] = {0xFF, 0xFF, 0xFF};
    const unsigned char neg_min_le[3] = {0x00, 0x00, 0x80};

    EXPECT_EQ (cast_24bit_to_int32_swap_order (pos_max_le), 8388607);
    EXPECT_EQ (cast_24bit_to_int32_swap_order (neg_one_le), -1);
    EXPECT_EQ (cast_24bit_to_int32_swap_order (neg_min_le), -8388608);
}

TEST (CustomCastTest, Cast16BitToInt32_BoundaryAndSignedValues_ReturnsExpectedTwosComplement)
{
    const unsigned char zero[2] = {0x00, 0x00};
    const unsigned char pos_max[2] = {0x7F, 0xFF};
    const unsigned char neg_one[2] = {0xFF, 0xFF};
    const unsigned char neg_min[2] = {0x80, 0x00};

    EXPECT_EQ (cast_16bit_to_int32 (zero), 0);
    EXPECT_EQ (cast_16bit_to_int32 (pos_max), 32767);
    EXPECT_EQ (cast_16bit_to_int32 (neg_one), -1);
    EXPECT_EQ (cast_16bit_to_int32 (neg_min), -32768);
}

TEST (CustomCastTest, Cast16BitLittleEndianVariants_BoundaryValues_ReturnExpectedValues)
{
    const unsigned char pos_max_le[2] = {0xFF, 0x7F};
    const unsigned char neg_one_le[2] = {0xFF, 0xFF};
    const unsigned char neg_min_le[2] = {0x00, 0x80};

    EXPECT_EQ (cast_16bit_to_int32_swap_order (pos_max_le), 32767);
    EXPECT_EQ (cast_16bit_to_int32_swap_order (neg_one_le), -1);
    EXPECT_EQ (cast_16bit_to_int32_swap_order (neg_min_le), -32768);

    EXPECT_EQ (cast_16bit_to_int16_little_endian (pos_max_le), 32767);
    EXPECT_EQ (cast_16bit_to_int16_little_endian (neg_one_le), -1);
    EXPECT_EQ (cast_16bit_to_int16_little_endian (neg_min_le), -32768);

    EXPECT_EQ (cast_16bit_to_uint16_little_endian (pos_max_le), 32767U);
    EXPECT_EQ (cast_16bit_to_uint16_little_endian (neg_one_le), 65535U);
    EXPECT_EQ (cast_16bit_to_uint16_little_endian (neg_min_le), 32768U);
}

TEST (CustomCastTest, Cast32BitToUint32LittleEndian_BytePattern_ReturnsExpectedUint32)
{
    const unsigned char bytes[4] = {0x78, 0x56, 0x34, 0x12};
    EXPECT_EQ (cast_32bit_to_uint32_little_endian (bytes), 0x12345678U);
}

TEST (CustomCastTest, Cast13BitAnd15BitToInt32_BoundaryValues_ReturnExpectedTwosComplement)
{
    const unsigned char pos_max_13[2] = {0x0F, 0xFF};
    const unsigned char neg_one_13[2] = {0x1F, 0xFF};
    const unsigned char neg_min_13[2] = {0x10, 0x00};

    EXPECT_EQ (cast_13bit_to_int32 (pos_max_13), 4095);
    EXPECT_EQ (cast_13bit_to_int32 (neg_one_13), -1);
    EXPECT_EQ (cast_13bit_to_int32 (neg_min_13), -4096);

    const unsigned char pos_max_15[2] = {0x3F, 0xFF};
    const unsigned char neg_one_15[2] = {0x7F, 0xFF};
    const unsigned char neg_min_15[2] = {0x40, 0x00};

    EXPECT_EQ (cast_15bit_to_int32 (pos_max_15), 16383);
    EXPECT_EQ (cast_15bit_to_int32 (neg_one_15), -1);
    EXPECT_EQ (cast_15bit_to_int32 (neg_min_15), -16384);
}

TEST (CustomCastTest, ExtractLsbBitsAndSwapEndians_BitPatterns_ReturnExpectedValues)
{
    const unsigned char packed[3] = {0xA5, 0x5A, 0xF0};
    EXPECT_EQ (extract_lsb_bits (packed, 0, 8), 0xA5U);
    EXPECT_EQ (extract_lsb_bits (packed, 4, 12), 0x5AAU);
    EXPECT_EQ (swap_endians (0x12345678), 0x78563412);
}
