#include "simplebluez/interfaces/GattManager1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<GattManager1> GattManager1::registry{
    "org.bluez.GattManager1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<GattManager1>(conn, proxy));
    }
    // clang-format on
};

GattManager1::GattManager1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.GattManager1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
GattManager1::~GattManager1() = default;

void GattManager1::RegisterApplication(std::string application_path) {
    SimpleDBus::Holder properties = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();

    // NOTE: The current documentation doesn't specify any options. Using a placeholder for now.

    auto msg = create_method_call("RegisterApplication");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(application_path), "o");
    msg.append_argument(properties, "a{sv}");

    _conn->send_with_reply(msg);
}

void GattManager1::UnregisterApplication(std::string application_path) {
    auto msg = create_method_call("UnregisterApplication");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(application_path), "o");
    _conn->send_with_reply(msg);
}