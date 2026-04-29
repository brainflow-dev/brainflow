#include "simplebluez/interfaces/LEAdvertisement1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<LEAdvertisement1> LEAdvertisement1::registry{
    "org.bluez.LEAdvertisement1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<LEAdvertisement1>(conn, proxy));
    }
    // clang-format on
};

LEAdvertisement1::LEAdvertisement1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.LEAdvertisement1") {
}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
LEAdvertisement1::~LEAdvertisement1() = default;

void LEAdvertisement1::message_handle(SimpleDBus::Message& msg) {
    if (msg.is_method_call(_interface_name, "Release")) {
        OnRelease();

        SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);
        _conn->send(reply);

    }
}