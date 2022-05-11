#include <gtest/gtest.h>

#include <simpledbus/base/Holder.h>

#include <any>
#include <map>
#include <string>
#include <variant>

using namespace SimpleDBus;

TEST(Holder, Boolean) {
    Holder h = Holder::create_boolean(true);
    EXPECT_EQ(h.get_boolean(), true);

    EXPECT_EQ(h.type(), Holder::Type::BOOLEAN);

    EXPECT_EQ(h.signature(), "b");

    EXPECT_EQ(h.represent(), "true\n");
}

TEST(Holder, Byte) {
    Holder h = Holder::create_byte(0x12);
    EXPECT_EQ(h.get_byte(), 0x12);

    EXPECT_EQ(h.type(), Holder::Type::BYTE);

    EXPECT_EQ(h.signature(), "y");

    EXPECT_EQ(h.represent(), "18\n");
}

TEST(Holder, Int16) {
    Holder h = Holder::create_int16(0x1234);
    EXPECT_EQ(h.get_int16(), 0x1234);

    EXPECT_EQ(h.type(), Holder::Type::INT16);

    EXPECT_EQ(h.signature(), "n");

    EXPECT_EQ(h.represent(), "4660\n");
}

TEST(Holder, Int32) {
    Holder h = Holder::create_int32(0x12345678);
    EXPECT_EQ(h.get_int32(), 0x12345678);

    EXPECT_EQ(h.type(), Holder::Type::INT32);

    EXPECT_EQ(h.signature(), "i");

    EXPECT_EQ(h.represent(), "305419896\n");
}

TEST(Holder, Int64) {
    Holder h = Holder::create_int64(0x123456789abcdef0);
    EXPECT_EQ(h.get_int64(), 0x123456789abcdef0);

    EXPECT_EQ(h.type(), Holder::Type::INT64);

    EXPECT_EQ(h.signature(), "x");

    EXPECT_EQ(h.represent(), "1311768467463790320\n");
}

TEST(Holder, Double) {
    Holder h = Holder::create_double(3.14);
    EXPECT_EQ(h.get_double(), 3.14);

    EXPECT_EQ(h.type(), Holder::Type::DOUBLE);

    EXPECT_EQ(h.signature(), "d");

    EXPECT_EQ(h.represent(), "3.14\n");
}

TEST(Holder, String) {
    Holder h = Holder::create_string("Hello, world!");
    EXPECT_EQ(h.get_string(), "Hello, world!");

    EXPECT_EQ(h.type(), Holder::Type::STRING);

    EXPECT_EQ(h.signature(), "s");

    EXPECT_EQ(h.represent(), "Hello, world!\n");
}

TEST(Holder, ObjectPath) {
    Holder h = Holder::create_object_path("/foo/bar");
    EXPECT_EQ(h.get_object_path(), "/foo/bar");

    EXPECT_EQ(h.type(), Holder::Type::OBJ_PATH);

    EXPECT_EQ(h.signature(), "o");

    EXPECT_EQ(h.represent(), "/foo/bar\n");
}

TEST(Holder, Signature) {
    Holder h = Holder::create_signature("s");
    EXPECT_EQ(h.get_signature(), "s");

    EXPECT_EQ(h.type(), Holder::Type::SIGNATURE);

    EXPECT_EQ(h.signature(), "g");

    EXPECT_EQ(h.represent(), "s\n");
}

TEST(Holder, ArrayHomogeneous) {
    Holder h = Holder::create_array();
    h.array_append(Holder::create_int32(42));

    EXPECT_EQ(h.get_array().size(), 1);
    EXPECT_EQ(h.get_array()[0].get_int32(), 42);

    EXPECT_EQ(h.type(), Holder::Type::ARRAY);

    EXPECT_EQ(h.signature(), "ai");

    EXPECT_EQ(h.represent(), "Array:\n  42\n");
}

TEST(Holder, ArrayHeterogeneous) {
    Holder h = Holder::create_array();
    h.array_append(Holder::create_int32(42));
    h.array_append(Holder::create_string("Hello, world!"));

    EXPECT_EQ(h.get_array().size(), 2);

    EXPECT_EQ(h.type(), Holder::Type::ARRAY);

    EXPECT_EQ(h.signature(), "av");

    EXPECT_EQ(h.represent(), "Array:\n  42\n  Hello, world!\n");
}

TEST(Holder, DictionaryHomogeneousString) {
    Holder h = Holder::create_dict();

    // Add three objects with string keys to the dictionary.
    h.dict_append(Holder::Type::STRING, "string_key1", Holder::create_string("value1"));
    h.dict_append(Holder::Type::STRING, "string_key2", Holder::create_string("value2"));
    h.dict_append(Holder::Type::STRING, "string_key3", Holder::create_string("value3"));

    EXPECT_EQ(h.type(), Holder::Type::DICT);

    EXPECT_EQ(h.signature(), "a{ss}");

    EXPECT_EQ(h.represent(), "Dictionary:\nstring_key1:\n  value1\nstring_key2:\n  value2\nstring_key3:\n  value3\n");
}

TEST(Holder, DictionaryHeterogeneous) {
    Holder h = Holder::create_dict();

    // Add three objects with string keys to the dictionary.
    h.dict_append(Holder::Type::STRING, "string_key1", Holder());
    h.dict_append(Holder::Type::STRING, "string_key2", Holder());
    h.dict_append(Holder::Type::STRING, "string_key3", Holder());

    // Add three objects with int keys to the dictionary.
    h.dict_append(Holder::Type::INT32, static_cast<int32_t>(123), Holder());
    h.dict_append(Holder::Type::INT32, static_cast<int32_t>(456), Holder());
    h.dict_append(Holder::Type::INT32, static_cast<int32_t>(789), Holder());

    // Check that all objects with string keys are returned.
    auto dict_string = h.get_dict_string();
    EXPECT_EQ(dict_string.size(), 3);
    EXPECT_EQ(dict_string.count("string_key1"), 1);
    EXPECT_EQ(dict_string.count("string_key2"), 1);
    EXPECT_EQ(dict_string.count("string_key3"), 1);

    // Check that all objects with int32 keys are returned.
    auto dict_int32 = h.get_dict_int32();
    EXPECT_EQ(dict_int32.size(), 3);
    EXPECT_EQ(dict_int32.count(123), 1);
    EXPECT_EQ(dict_int32.count(456), 1);
    EXPECT_EQ(dict_int32.count(789), 1);

    // TODO: Expand this test to check with all remaining types.
}

// TODO: Add tests for equality comparison of Holders.
