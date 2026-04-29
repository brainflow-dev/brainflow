#include "simplebluez/interfaces/GattCharacteristic1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<GattCharacteristic1> GattCharacteristic1::registry{
    "org.bluez.GattCharacteristic1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<GattCharacteristic1>(conn, proxy));
    }
    // clang-format on
};

GattCharacteristic1::GattCharacteristic1(std::shared_ptr<SimpleDBus::Connection> conn,
                                         std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.GattCharacteristic1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
GattCharacteristic1::~GattCharacteristic1() = default;

void GattCharacteristic1::StartNotify() {
    auto msg = create_method_call("StartNotify");
    _conn->send_with_reply(msg);
}

void GattCharacteristic1::StopNotify() {
    auto msg = create_method_call("StopNotify");
    _conn->send_with_reply(msg);
}

void GattCharacteristic1::WriteValue(const ByteArray& value, WriteType type) {
    SimpleDBus::Holder value_data = SimpleDBus::Holder::create<std::vector<SimpleDBus::Holder>>();
    for (size_t i = 0; i < value.size(); i++) {
        value_data.array_append(SimpleDBus::Holder::create<uint8_t>(value[i]));
    }

    SimpleDBus::Holder options = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();
    if (type == WriteType::REQUEST) {
        options.dict_append(SimpleDBus::Holder::Type::STRING, "type",
                            SimpleDBus::Holder::create<std::string>("request"));
    } else if (type == WriteType::COMMAND) {
        options.dict_append(SimpleDBus::Holder::Type::STRING, "type",
                            SimpleDBus::Holder::create<std::string>("command"));
    }

    auto msg = create_method_call("WriteValue");
    msg.append_argument(value_data, "ay");
    msg.append_argument(options, "a{sv}");
    _conn->send_with_reply(msg);
}

ByteArray GattCharacteristic1::ReadValue() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBus::Holder options = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();
    msg.append_argument(options, "a{sv}");

    SimpleDBus::Message reply_msg = _conn->send_with_reply(msg);
    SimpleDBus::Holder value = reply_msg.extract();

    Value.set(value);
    return Value();
}

void GattCharacteristic1::message_handle(SimpleDBus::Message& msg) {
    if (msg.is_method_call(_interface_name, "ReadValue")) {
        SimpleDBus::Holder options = msg.extract();

        OnReadValue();

        SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);
        reply.append_argument(_properties["Value"]->get(), "ay");
        _conn->send(reply);
    } else if (msg.is_method_call(_interface_name, "WriteValue")) {
        SimpleDBus::Holder value = msg.extract();
        msg.extract_next();
        SimpleDBus::Holder options = msg.extract();

        Value.set(value);
        SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);
        _conn->send(reply);

        OnWriteValue(Value.get());
    } else if (msg.is_method_call(_interface_name, "StartNotify")) {
        SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);
        _conn->send(reply);

        Notifying.set(true).emit();

        OnStartNotify();
    } else if (msg.is_method_call(_interface_name, "StopNotify")) {
        SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);
        _conn->send(reply);

        Notifying.set(false).emit();

        OnStopNotify();
    }
}