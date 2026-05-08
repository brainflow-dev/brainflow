#include "simplebluez/interfaces/Battery1.h"

#include <iostream>

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<Battery1> Battery1::registry{
    "org.bluez.Battery1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Battery1>(conn, proxy));
    }
    // clang-format on
};

Battery1::Battery1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.Battery1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
Battery1::~Battery1() = default;

