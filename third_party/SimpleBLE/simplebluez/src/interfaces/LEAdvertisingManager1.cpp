#include "simplebluez/interfaces/LEAdvertisingManager1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<LEAdvertisingManager1> LEAdvertisingManager1::registry{
    "org.bluez.LEAdvertisingManager1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<LEAdvertisingManager1>(conn, proxy));
    }
    // clang-format on
};

LEAdvertisingManager1::LEAdvertisingManager1(std::shared_ptr<SimpleDBus::Connection> conn,
                                             std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.LEAdvertisingManager1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
LEAdvertisingManager1::~LEAdvertisingManager1() = default;

void LEAdvertisingManager1::RegisterAdvertisement(std::string advertisement_path) {
    SimpleDBus::Holder properties = SimpleDBus::Holder::create<std::map<std::string, SimpleDBus::Holder>>();

    // NOTE: The current documentation doesn't specify any options. Using a placeholder for now.

    auto msg = create_method_call("RegisterAdvertisement");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(advertisement_path), "o");
    msg.append_argument(properties, "a{sv}");

    _conn->send_with_reply(msg);
}

void LEAdvertisingManager1::UnregisterAdvertisement(std::string advertisement_path) {
    auto msg = create_method_call("UnregisterAdvertisement");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(advertisement_path), "o");
    _conn->send_with_reply(msg);
}
