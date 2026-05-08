#include <gtest/gtest.h>

#include <simpledbus/base/Holder.h>

#include <any>
#include <map>
#include <string>

using namespace SimpleDBus;

TEST(Holder, Boolean) {
    Holder h = Holder::create<bool>(true);
    EXPECT_EQ(h.get<bool>(), true);

    EXPECT_EQ(h.type(), Holder::Type::BOOLEAN);

    EXPECT_EQ(h.signature(), "b");

    EXPECT_EQ(h.represent(), "true\n");
}

TEST(Holder, Byte) {
    Holder h = Holder::create<uint8_t>(0x12);
    EXPECT_EQ(h.get<uint8_t>(), 0x12);

    EXPECT_EQ(h.type(), Holder::Type::BYTE);

    EXPECT_EQ(h.signature(), "y");

    EXPECT_EQ(h.represent(), "18\n");
}

TEST(Holder, Int16) {
    Holder h = Holder::create<int16_t>(0x1234);
    EXPECT_EQ(h.get<int16_t>(), 0x1234);

    EXPECT_EQ(h.type(), Holder::Type::INT16);

    EXPECT_EQ(h.signature(), "n");

    EXPECT_EQ(h.represent(), "4660\n");
}

TEST(Holder, Int32) {
    Holder h = Holder::create<int32_t>(0x12345678);
    EXPECT_EQ(h.get<int32_t>(), 0x12345678);

    EXPECT_EQ(h.type(), Holder::Type::INT32);

    EXPECT_EQ(h.signature(), "i");

    EXPECT_EQ(h.represent(), "305419896\n");
}

TEST(Holder, Int64) {
    Holder h = Holder::create<int64_t>(0x123456789abcdef0);
    EXPECT_EQ(h.get<int64_t>(), 0x123456789abcdef0);

    EXPECT_EQ(h.type(), Holder::Type::INT64);

    EXPECT_EQ(h.signature(), "x");

    EXPECT_EQ(h.represent(), "1311768467463790320\n");
}

TEST(Holder, Double) {
    Holder h = Holder::create<double>(3.14);
    EXPECT_EQ(h.get<double>(), 3.14);

    EXPECT_EQ(h.type(), Holder::Type::DOUBLE);

    EXPECT_EQ(h.signature(), "d");

    EXPECT_EQ(h.represent(), "3.14\n");
}

TEST(Holder, String) {
    Holder h = Holder::create<std::string>("Hello, world!");
    EXPECT_EQ(h.get<std::string>(), "Hello, world!");

    EXPECT_EQ(h.type(), Holder::Type::STRING);

    EXPECT_EQ(h.signature(), "s");

    EXPECT_EQ(h.represent(), "Hello, world!\n");
}

TEST(Holder, ObjectPath) {
    Holder h = Holder::create<ObjectPath>("/foo/bar");
    EXPECT_EQ(h.get<ObjectPath>(), ObjectPath("/foo/bar"));

    EXPECT_EQ(h.type(), Holder::Type::OBJ_PATH);

    EXPECT_EQ(h.signature(), "o");

    EXPECT_EQ(h.represent(), "/foo/bar\n");
}

TEST(Holder, Signature) {
    Holder h = Holder::create<Signature>("s");
    EXPECT_EQ(h.get<Signature>(), Signature("s"));

    EXPECT_EQ(h.type(), Holder::Type::SIGNATURE);

    EXPECT_EQ(h.signature(), "g");

    EXPECT_EQ(h.represent(), "s\n");
}

TEST(Holder, ArrayHomogeneous) {
    Holder h = Holder::create<std::vector<Holder>>();
    h.array_append(Holder::create<int32_t>(42));

    EXPECT_EQ(h.get<std::vector<Holder>>().size(), 1);
    EXPECT_EQ(h.get<std::vector<Holder>>()[0].get<int32_t>(), 42);

    EXPECT_EQ(h.type(), Holder::Type::ARRAY);

    EXPECT_EQ(h.signature(), "ai");

    EXPECT_EQ(h.represent(), "Array:\n  42\n");
}

TEST(Holder, ArrayHeterogeneous) {
    Holder h = Holder::create<std::vector<Holder>>();
    h.array_append(Holder::create<int32_t>(42));
    h.array_append(Holder::create<std::string>("Hello, world!"));

    EXPECT_EQ(h.get<std::vector<Holder>>().size(), 2);

    EXPECT_EQ(h.type(), Holder::Type::ARRAY);

    EXPECT_EQ(h.signature(), "av");

    EXPECT_EQ(h.represent(), "Array:\n  42\n  Hello, world!\n");
}

TEST(Holder, DictionaryHomogeneousString) {
    Holder h = Holder::create<std::map<std::string, Holder>>();

    // Add three objects with string keys to the dictionary.
    h.dict_append(Holder::Type::STRING, "string_key1", Holder::create<std::string>("value1"));
    h.dict_append(Holder::Type::STRING, "string_key2", Holder::create<std::string>("value2"));
    h.dict_append(Holder::Type::STRING, "string_key3", Holder::create<std::string>("value3"));

    EXPECT_EQ(h.type(), Holder::Type::DICT);

    EXPECT_EQ(h.signature(), "a{ss}");

    EXPECT_EQ(h.represent(), "Dictionary:\nstring_key1:\n  value1\nstring_key2:\n  value2\nstring_key3:\n  value3\n");
}

TEST(Holder, DictionaryHeterogeneous) {
    Holder h = Holder::create<std::map<std::string, Holder>>();

    // Add three objects with string keys to the dictionary.
    h.dict_append(Holder::Type::STRING, "string_key1", Holder());
    h.dict_append(Holder::Type::STRING, "string_key2", Holder());
    h.dict_append(Holder::Type::STRING, "string_key3", Holder());

    // Add three objects with int keys to the dictionary.
    h.dict_append(Holder::Type::INT32, static_cast<int32_t>(123), Holder());
    h.dict_append(Holder::Type::INT32, static_cast<int32_t>(456), Holder());
    h.dict_append(Holder::Type::INT32, static_cast<int32_t>(789), Holder());

    // Check that all objects with string keys are returned.
    auto dict_string = h.get<std::map<std::string, Holder>>();
    EXPECT_EQ(dict_string.size(), 3);
    EXPECT_EQ(dict_string.count("string_key1"), 1);
    EXPECT_EQ(dict_string.count("string_key2"), 1);
    EXPECT_EQ(dict_string.count("string_key3"), 1);

    // Check that all objects with int32 keys are returned.
    auto dict_int32 = h.get<std::map<int32_t, Holder>>();
    EXPECT_EQ(dict_int32.size(), 3);
    EXPECT_EQ(dict_int32.count(123), 1);
    EXPECT_EQ(dict_int32.count(456), 1);
    EXPECT_EQ(dict_int32.count(789), 1);

    // TODO: Expand this test to check with all remaining types.
}

// TODO: Add tests for equality comparison of Holders.
