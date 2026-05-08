#include "simplebluez/interfaces/Device1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<Device1> Device1::registry{
    "org.bluez.Device1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Device1>(conn, proxy));
    }
    // clang-format on
};

Device1::Device1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.Device1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
Device1::~Device1() = default;

void Device1::Connect() {
    auto msg = create_method_call("Connect");
    _conn->send_with_reply(msg);
}

void Device1::Disconnect() {
    auto msg = create_method_call("Disconnect");
    _conn->send_with_reply(msg);
}

void Device1::Pair() {
    auto msg = create_method_call("Pair");
    _conn->send_with_reply(msg);
}

void Device1::CancelPairing() {
    auto msg = create_method_call("CancelPairing");
    _conn->send_with_reply(msg);
}