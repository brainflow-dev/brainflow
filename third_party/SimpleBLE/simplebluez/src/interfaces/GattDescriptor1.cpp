#include "simplebluez/interfaces/GattDescriptor1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<GattDescriptor1> GattDescriptor1::registry{
    "org.bluez.GattDescriptor1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<GattDescriptor1>(conn, proxy));
    }
    // clang-format on
};

GattDescriptor1::GattDescriptor1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.GattDescriptor1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
GattDescriptor1::~GattDescriptor1() = default;

void GattDescriptor1::WriteValue(const ByteArray& value) {
    SimpleDBus::Holder value_data = SimpleDBus::Holder::create<std::vector<SimpleDBus::Holder>>();
    for (size_t i = 0; i < value.size(); i++) {
        value_data.array_append(SimpleDBus::Holder::create<uint8_t>(value[i]));
    }

    SimpleDBus::Holder options = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();

    auto msg = create_method_call("WriteValue");
    msg.append_argument(value_data, "ay");
    msg.append_argument(options, "a{sv}");
    _conn->send_with_reply(msg);
}

ByteArray GattDescriptor1::ReadValue() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBus::Holder options = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();
    msg.append_argument(options, "a{sv}");

    SimpleDBus::Message reply_msg = _conn->send_with_reply(msg);
    SimpleDBus::Holder value = reply_msg.extract();
    Value.set(value);

    return Value();
}
