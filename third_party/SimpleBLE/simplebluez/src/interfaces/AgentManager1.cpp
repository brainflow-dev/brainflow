#include <simplebluez/interfaces/AgentManager1.h>

using namespace SimpleBluez;

#include <iostream>

AgentManager1::AgentManager1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path)
    : SimpleDBus::Interface(conn, "org.bluez", path, "org.bluez.AgentManager1") {}

void AgentManager1::RegisterAgent(std::string agent, std::string capability) {
    auto msg = create_method_call("RegisterAgent");
    msg.append_argument(SimpleDBus::Holder::create_object_path(agent), "o");
    msg.append_argument(SimpleDBus::Holder::create_string(capability), "s");
    _conn->send_with_reply_and_block(msg);
}

void AgentManager1::RequestDefaultAgent(std::string agent) {
    auto msg = create_method_call("RequestDefaultAgent");
    msg.append_argument(SimpleDBus::Holder::create_object_path(agent), "o");
    _conn->send_with_reply_and_block(msg);
}

void AgentManager1::UnregisterAgent(std::string agent) {
    auto msg = create_method_call("UnregisterAgent");
    msg.append_argument(SimpleDBus::Holder::create_object_path(agent), "o");
    _conn->send_with_reply_and_block(msg);
}
