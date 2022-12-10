#include <gtest/gtest.h>

#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Message.h>

#include <chrono>

using namespace SimpleDBus;

class MessageTest : public ::testing::Test {
  protected:
    void SetUp() override {
        conn = new Connection(DBUS_BUS_SESSION);
        conn->init();
    }

    void TearDown() override {
        conn->uninit();
        delete conn;
        conn = nullptr;
    }

    Connection* conn;
};

TEST_F(MessageTest, SendReceiveBooleanTrue) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveBoolean");

    msg.append_argument(Holder::create_boolean(true), DBUS_TYPE_BOOLEAN_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::BOOLEAN);
    EXPECT_EQ(h_reply.get_boolean(), true);
}

TEST_F(MessageTest, SendReceiveBooleanFalse) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveBoolean");

    msg.append_argument(Holder::create_boolean(false), DBUS_TYPE_BOOLEAN_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::BOOLEAN);
    EXPECT_EQ(h_reply.get_boolean(), false);
}

TEST_F(MessageTest, SendReceiveByte) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveByte");

    uint8_t value = 0x42;
    msg.append_argument(Holder::create_byte(value), DBUS_TYPE_BYTE_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::BYTE);
    EXPECT_EQ(h_reply.get_byte(), value);
}

TEST_F(MessageTest, SendReceiveInt16) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveInt16");

    int16_t value = 0x1234;
    msg.append_argument(Holder::create_int16(value), DBUS_TYPE_INT16_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::INT16);
    EXPECT_EQ(h_reply.get_int16(), value);
}

TEST_F(MessageTest, SendReceiveUint16) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveUint16");

    uint16_t value = 0x1234;
    msg.append_argument(Holder::create_uint16(value), DBUS_TYPE_UINT16_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::UINT16);
    EXPECT_EQ(h_reply.get_uint16(), value);
}

TEST_F(MessageTest, SendReceiveInt32) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveInt32");

    int32_t value = 0x12345678;
    msg.append_argument(Holder::create_int32(value), DBUS_TYPE_INT32_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::INT32);
    EXPECT_EQ(h_reply.get_int32(), value);
}

TEST_F(MessageTest, SendReceiveUint32) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveUint32");

    uint32_t value = 0x12345678;
    msg.append_argument(Holder::create_uint32(value), DBUS_TYPE_UINT32_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::UINT32);
    EXPECT_EQ(h_reply.get_uint32(), value);
}

TEST_F(MessageTest, SendReceiveInt64) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveInt64");

    int64_t value = 0x1234567812345678;
    msg.append_argument(Holder::create_int64(value), DBUS_TYPE_INT64_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::INT64);
    EXPECT_EQ(h_reply.get_int64(), value);
}

TEST_F(MessageTest, SendReceiveUint64) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveUint64");

    uint64_t value = 0x1234567812345678;
    msg.append_argument(Holder::create_uint64(value), DBUS_TYPE_UINT64_AS_STRING);

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::UINT64);
    EXPECT_EQ(h_reply.get_uint64(), value);
}

TEST_F(MessageTest, SendReceiveArrayInt32) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveArrayInt32");

    Holder h_msg = Holder::create_array();
    h_msg.array_append(Holder::create_int32(0x12345678));
    h_msg.array_append(Holder::create_int32(0x87654321));
    msg.append_argument(h_msg, "ai");

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::ARRAY);

    std::vector<Holder> array = h_reply.get_array();

    ASSERT_EQ(array.size(), 2);

    EXPECT_EQ(array[0].type(), Holder::Type::INT32);
    EXPECT_EQ(array[0].get_int32(), 0x12345678);

    EXPECT_EQ(array[1].type(), Holder::Type::INT32);
    EXPECT_EQ(array[1].get_int32(), 0x87654321);
}

TEST_F(MessageTest, SendReceiveArrayString) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveArrayString");

    Holder h_msg = Holder::create_array();
    h_msg.array_append(Holder::create_string("Hello"));
    h_msg.array_append(Holder::create_string("World"));
    msg.append_argument(h_msg, "as");

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::ARRAY);

    std::vector<Holder> array = h_reply.get_array();

    ASSERT_EQ(array.size(), 2);

    EXPECT_EQ(array[0].type(), Holder::Type::STRING);
    EXPECT_EQ(array[0].get_string(), "Hello");

    EXPECT_EQ(array[1].type(), Holder::Type::STRING);
    EXPECT_EQ(array[1].get_string(), "World");
}

TEST_F(MessageTest, SendReceiveDictInt32) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveDictInt32");

    Holder h_msg = Holder::create_dict();
    h_msg.dict_append(Holder::INT32, static_cast<int32_t>(0x12345678), Holder::create_int32(0x87654321));
    h_msg.dict_append(Holder::INT32, static_cast<int32_t>(0x87654321), Holder::create_string("Hello"));
    msg.append_argument(h_msg, "a{iv}");

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::DICT);

    std::map<int32_t, Holder> dict = h_reply.get_dict_int32();

    ASSERT_EQ(dict.size(), 2);
    ASSERT_EQ(dict.count(0x12345678), 1);
    ASSERT_EQ(dict.count(0x87654321), 1);

    EXPECT_EQ(dict[0x12345678].type(), Holder::Type::INT32);
    EXPECT_EQ(dict[0x12345678].get_int32(), 0x87654321);

    EXPECT_EQ(dict[0x87654321].type(), Holder::Type::STRING);
    EXPECT_EQ(dict[0x87654321].get_string(), "Hello");
}

TEST_F(MessageTest, SendReceiveDictString) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "SendReceiveDictString");

    Holder h_msg = Holder::create_dict();
    h_msg.dict_append(Holder::STRING, "key1", Holder::create_int32(0x87654321));
    h_msg.dict_append(Holder::STRING, "key2", Holder::create_string("Hello"));
    msg.append_argument(h_msg, "a{sv}");

    Message reply = conn->send_with_reply_and_block(msg);
    Holder h_reply = reply.extract();

    EXPECT_EQ(h_reply.type(), Holder::Type::DICT);

    std::map<std::string, Holder> dict = h_reply.get_dict_string();

    ASSERT_EQ(dict.size(), 2);
    ASSERT_EQ(dict.count("key1"), 1);
    ASSERT_EQ(dict.count("key2"), 1);

    EXPECT_EQ(dict["key1"].type(), Holder::Type::INT32);
    EXPECT_EQ(dict["key1"].get_int32(), 0x87654321);

    EXPECT_EQ(dict["key2"].type(), Holder::Type::STRING);
    EXPECT_EQ(dict["key2"].get_string(), "Hello");
}

TEST_F(MessageTest, ReceiveMethodCallSuccess) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "TriggerMethodCall");

    msg.append_argument(Holder::create_string(conn->unique_name()), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("/my/custom/path"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("my.interface"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("MyMethod"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("Hello World"), DBUS_TYPE_STRING_AS_STRING);
    conn->send_with_reply_and_block(msg);

    // Wait for the method call to be received
    bool method_called = false;
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now() + std::chrono::seconds(1);
    while (std::chrono::system_clock::now() < end) {
        conn->read_write();
        auto method_call = conn->pop_message();
        if (method_call.is_valid()) {
            EXPECT_EQ(method_call.get_path(), "/my/custom/path");
            EXPECT_EQ(method_call.get_interface(), "my.interface");
            EXPECT_EQ(method_call.get_member(), "MyMethod");

            Holder arguments = method_call.extract();
            EXPECT_EQ(arguments.type(), Holder::Type::STRING);
            EXPECT_EQ(arguments.get_string(), "Hello World");

            Message reply = Message::create_method_return(method_call);
            reply.append_argument(Holder::create_string("Nice to meet you"), DBUS_TYPE_STRING_AS_STRING);
            conn->send(reply);
            method_called = true;
            break;
        }
    }
    EXPECT_TRUE(method_called);
}

TEST_F(MessageTest, ReceiveMethodCallFailure) {
    Message msg = Message::create_method_call("simpledbus.tester.python", "/", "simpledbus.tester.message",
                                              "TriggerMethodCall");

    msg.append_argument(Holder::create_string(conn->unique_name()), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("/my/custom/path"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("my.interface"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("MyMethod"), DBUS_TYPE_STRING_AS_STRING);
    msg.append_argument(Holder::create_string("Hello World"), DBUS_TYPE_STRING_AS_STRING);
    conn->send_with_reply_and_block(msg);

    // Wait for the method call to be received
    bool method_called = false;
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now() + std::chrono::seconds(1);
    while (std::chrono::system_clock::now() < end) {
        conn->read_write();
        auto method_call = conn->pop_message();
        if (method_call.is_valid()) {
            EXPECT_EQ(method_call.get_path(), "/my/custom/path");
            EXPECT_EQ(method_call.get_interface(), "my.interface");
            EXPECT_EQ(method_call.get_member(), "MyMethod");

            Holder arguments = method_call.extract();
            EXPECT_EQ(arguments.type(), Holder::Type::STRING);
            EXPECT_EQ(arguments.get_string(), "Hello World");

            Message reply = Message::create_error(method_call, "simpledbus.error.unknown", "Unknown error");
            conn->send(reply);
            method_called = true;
            break;
        }
    }
    EXPECT_TRUE(method_called);
}
