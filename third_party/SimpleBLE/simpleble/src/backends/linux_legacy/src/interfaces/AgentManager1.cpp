#include <simplebluezlegacy/interfaces/AgentManager1.h>

using namespace SimpleBluezLegacy;

const SimpleDBusLegacy::AutoRegisterInterface<AgentManager1> AgentManager1::registry{
    "org.bluez.AgentManager1",
    // clang-format off
    [](std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path, const SimpleDBusLegacy::Holder& options) -> std::shared_ptr<SimpleDBusLegacy::Interface> {
        return std::static_pointer_cast<SimpleDBusLegacy::Interface>(std::make_shared<AgentManager1>(conn, path));
    }
    // clang-format on
};

AgentManager1::AgentManager1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path)
    : SimpleDBusLegacy::Interface(conn, "org.bluez", path, "org.bluez.AgentManager1") {}

void AgentManager1::RegisterAgent(std::string agent, std::string capability) {
    auto msg = create_method_call("RegisterAgent");
    msg.append_argument(SimpleDBusLegacy::Holder::create_object_path(agent), "o");
    msg.append_argument(SimpleDBusLegacy::Holder::create_string(capability), "s");
    _conn->send_with_reply_and_block(msg);
}

void AgentManager1::RequestDefaultAgent(std::string agent) {
    auto msg = create_method_call("RequestDefaultAgent");
    msg.append_argument(SimpleDBusLegacy::Holder::create_object_path(agent), "o");
    _conn->send_with_reply_and_block(msg);
}

void AgentManager1::UnregisterAgent(std::string agent) {
    auto msg = create_method_call("UnregisterAgent");
    msg.append_argument(SimpleDBusLegacy::Holder::create_object_path(agent), "o");
    _conn->send_with_reply_and_block(msg);
}
