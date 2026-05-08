#include <simplebluez/interfaces/AgentManager1.h>

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<AgentManager1> AgentManager1::registry{
    "org.bluez.AgentManager1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<AgentManager1>(conn, proxy));
    }
    // clang-format on
};

AgentManager1::AgentManager1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.AgentManager1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
AgentManager1::~AgentManager1() = default;

void AgentManager1::RegisterAgent(std::string agent, std::string capability) {
    auto msg = create_method_call("RegisterAgent");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(agent), "o");
    msg.append_argument(SimpleDBus::Holder::create<std::string>(capability), "s");
    _conn->send_with_reply(msg);
}

void AgentManager1::RequestDefaultAgent(std::string agent) {
    auto msg = create_method_call("RequestDefaultAgent");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(agent), "o");
    _conn->send_with_reply(msg);
}

void AgentManager1::UnregisterAgent(std::string agent) {
    auto msg = create_method_call("UnregisterAgent");
    msg.append_argument(SimpleDBus::Holder::create<SimpleDBus::ObjectPath>(agent), "o");
    _conn->send_with_reply(msg);
}
