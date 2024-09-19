#include <gtest/gtest.h>
#include "external/kvn_bytearray.h"

using namespace kvn;

TEST(ByteArrayTest, DefaultConstructor) {
    bytearray byteArray;
    EXPECT_EQ(byteArray.size(), 0);
}

TEST(ByteArrayTest, VectorConstructor) {
    std::vector<uint8_t> vec = {1, 2, 3, 4};
    bytearray byteArray(vec);
    EXPECT_EQ(byteArray.size(), 4);
    EXPECT_EQ(byteArray[0], 1);
    EXPECT_EQ(byteArray[1], 2);
    EXPECT_EQ(byteArray[2], 3);
    EXPECT_EQ(byteArray[3], 4);
}

TEST(ByteArrayTest, PointerConstructor) {
    uint8_t data[] = {1, 2, 3, 4};
    bytearray byteArray(data, 4);
    EXPECT_EQ(byteArray.size(), 4);
    EXPECT_EQ(byteArray[0], 1);
    EXPECT_EQ(byteArray[1], 2);
    EXPECT_EQ(byteArray[2], 3);
    EXPECT_EQ(byteArray[3], 4);
}

TEST(ByteArrayTest, StringConstructor) {
    std::string str = "Hello";
    bytearray byteArray(str);
    EXPECT_EQ(byteArray.size(), 5);
    EXPECT_EQ(byteArray[0], 'H');
    EXPECT_EQ(byteArray[1], 'e');
    EXPECT_EQ(byteArray[2], 'l');
    EXPECT_EQ(byteArray[3], 'l');
    EXPECT_EQ(byteArray[4], 'o');
}

TEST(ByteArrayTest, CharArrayConstructor) {
    const char str[] = {'H', 'e', 'l', 'l', 'o'};
    bytearray byteArray(str, 5);
    EXPECT_EQ(byteArray.size(), 5);
    EXPECT_EQ(byteArray[0], 'H');
    EXPECT_EQ(byteArray[1], 'e');
    EXPECT_EQ(byteArray[2], 'l');
    EXPECT_EQ(byteArray[3], 'l');
    EXPECT_EQ(byteArray[4], 'o');
}

TEST(ByteArrayTest, CStringConstructor) {
    const char* str = "Hello";
    bytearray byteArray(str);
    EXPECT_EQ(byteArray.size(), 5);
    EXPECT_EQ(byteArray[0], 'H');
    EXPECT_EQ(byteArray[1], 'e');
    EXPECT_EQ(byteArray[2], 'l');
    EXPECT_EQ(byteArray[3], 'l');
    EXPECT_EQ(byteArray[4], 'o');
}

TEST(ByteArrayTest, FromHexValid) {
    bytearray byteArray = bytearray::fromHex("48656c6C6f");
    EXPECT_EQ(byteArray.size(), 5);
    EXPECT_EQ(byteArray[0], 'H');
    EXPECT_EQ(byteArray[1], 'e');
    EXPECT_EQ(byteArray[2], 'l');
    EXPECT_EQ(byteArray[3], 'l');
    EXPECT_EQ(byteArray[4], 'o');
}

TEST(ByteArrayTest, FromHexValidWithPrefix) {
    bytearray byteArray = bytearray::fromHex("0x48656c6C6f");
    EXPECT_EQ(byteArray.size(), 5);
    EXPECT_EQ(byteArray[0], 'H');
    EXPECT_EQ(byteArray[1], 'e');
    EXPECT_EQ(byteArray[2], 'l');
    EXPECT_EQ(byteArray[3], 'l');
    EXPECT_EQ(byteArray[4], 'o');
}

TEST(ByteArrayTest, FromHexInvalid) {
    EXPECT_THROW(bytearray::fromHex("123"), std::length_error);
    EXPECT_THROW(bytearray::fromHex("G123"), std::invalid_argument);
}

TEST(ByteArrayTest, ToHex) {
    bytearray byteArray("Hello");
    EXPECT_EQ(byteArray.toHex(), "48656c6c6f");
    EXPECT_EQ(byteArray.toHex(true), "48 65 6c 6c 6f ");
}

TEST(ByteArrayTest, StingConversion) {
    bytearray byteArray = bytearray::fromHex("48656c6C6f");
    std::string str = static_cast<std::string>(byteArray);
    EXPECT_EQ(str, "Hello");
}

TEST(ByteArrayTest, StreamOperator) {
    bytearray byteArray("Hello");
    std::ostringstream oss;
    oss << byteArray;
    EXPECT_EQ(oss.str(), "48 65 6c 6c 6f ");
}

TEST(ByteArrayTest, DefaultConstructor_Empty) {
    bytearray byteArray;
    EXPECT_TRUE(byteArray.empty());
}

TEST(ByteArrayTest, PushBackIncreasesSize) {
    bytearray byteArray;
    byteArray.push_back(0x01);
    EXPECT_EQ(1, byteArray.size());
    byteArray.push_back(0x02);
    EXPECT_EQ(2, byteArray.size());
}

TEST(ByteArrayTest, ClearEmptiesArray) {
    bytearray byteArray;
    byteArray.push_back(0x01);
    byteArray.push_back(0x02);
    byteArray.clear();
    EXPECT_TRUE(byteArray.empty());
}

TEST(ByteArrayTest, IndexOperatorAccessesCorrectElement) {
    bytearray byteArray;
    byteArray.push_back(0x01);
    byteArray.push_back(0x02);
    EXPECT_EQ(0x01, byteArray[0]);
    EXPECT_EQ(0x02, byteArray[1]);
}

TEST(ByteArrayTest, DataPointerIsValid) {
    bytearray byteArray;
    byteArray.push_back(0x01);
    byteArray.push_back(0x02);
    const uint8_t* data_ptr = byteArray.data();
    ASSERT_NE(nullptr, data_ptr);
    EXPECT_EQ(0x01, data_ptr[0]);
    EXPECT_EQ(0x02, data_ptr[1]);
}

TEST(ByteArrayTest, BeginEndIterators) {
    bytearray byteArray;
    byteArray.push_back(0x01);
    byteArray.push_back(0x02);
    auto it = byteArray.begin();
    EXPECT_EQ(0x01, *it);
    ++it;
    EXPECT_EQ(0x02, *it);
    ++it;
    EXPECT_EQ(byteArray.end(), it);
}

TEST(ByteArrayTest, ConstIndexOperatorAccessesCorrectElement) {
    bytearray byteArray;
    byteArray.push_back(0x01);
    byteArray.push_back(0x02);
    const bytearray constByteArray = byteArray;
    EXPECT_EQ(0x01, constByteArray[0]);
    EXPECT_EQ(0x02, constByteArray[1]);
}

TEST(ByteArrayTest, SliceValidRange) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);
    bytearray slicedArray = byteArray.slice(1, 3);

    ASSERT_EQ(slicedArray.size(), 2);
    EXPECT_EQ(slicedArray[0], 0x02);
    EXPECT_EQ(slicedArray[1], 0x03);
}

TEST(ByteArrayTest, SliceFullRange) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);
    bytearray slicedArray = byteArray.slice(0, byteArray.size());

    ASSERT_EQ(slicedArray.size(), byteArray.size());
    EXPECT_EQ(slicedArray[0], 0x01);
    EXPECT_EQ(slicedArray[4], 0x05);
}

TEST(ByteArrayTest, SliceSingleElement) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);
    bytearray slicedArray = byteArray.slice(2, 3);

    ASSERT_EQ(slicedArray.size(), 1);
    EXPECT_EQ(slicedArray[0], 0x03);
}

TEST(ByteArrayTest, SliceOutOfRange) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);

    EXPECT_THROW(byteArray.slice(1, 6), std::out_of_range);
    EXPECT_THROW(byteArray.slice(6, 7), std::out_of_range);
}

TEST(ByteArrayTest, SliceInvalidRange) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);

    EXPECT_THROW(byteArray.slice(3, 2), std::out_of_range);
}

TEST(ByteArrayTest, SliceFromIndexToEnd) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);
    bytearray slicedArray = byteArray.slice_from(2);

    ASSERT_EQ(slicedArray.size(), 3);
    EXPECT_EQ(slicedArray[0], 0x03);
    EXPECT_EQ(slicedArray[1], 0x04);
    EXPECT_EQ(slicedArray[2], 0x05);
}

TEST(ByteArrayTest, SliceFromBeginningToIndex) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);
    bytearray slicedArray = byteArray.slice_to(3);

    ASSERT_EQ(slicedArray.size(), 3);
    EXPECT_EQ(slicedArray[0], 0x01);
    EXPECT_EQ(slicedArray[1], 0x02);
    EXPECT_EQ(slicedArray[2], 0x03);
}

TEST(ByteArrayTest, SliceFromIndexToEndOutOfRange) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);

    EXPECT_THROW(byteArray.slice_from(6), std::out_of_range);
}

TEST(ByteArrayTest, SliceFromBeginningToIndexOutOfRange) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    bytearray byteArray(vec);

    EXPECT_THROW(byteArray.slice_to(6), std::out_of_range);
}
