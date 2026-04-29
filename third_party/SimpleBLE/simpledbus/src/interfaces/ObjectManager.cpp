#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/interfaces/ObjectManager.h>

using namespace SimpleDBus;
using namespace SimpleDBus::Interfaces;

const AutoRegisterInterface<ObjectManager> ObjectManager::registry{
    "org.freedesktop.DBus.ObjectManager",
    // clang-format off
    [](std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy) -> std::shared_ptr<Interface> {
        return std::static_pointer_cast<Interface>(std::make_shared<ObjectManager>(conn, proxy));
    }
    // clang-format on
};

ObjectManager::ObjectManager(std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy)
    : Interface(conn, proxy, "org.freedesktop.DBus.ObjectManager") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
ObjectManager::~ObjectManager() = default;

Holder ObjectManager::GetManagedObjects() {
    Message query_msg = Message::create_method_call(_bus_name, _path, _interface_name, "GetManagedObjects");
    Message reply_msg = _conn->send_with_reply_and_block(query_msg);
    return reply_msg.extract();
}

// NOTE to future Kevin:
// There is a chance to further simplify the logic handed over to Proxy by moving the ownership of
// interfaces into this class and have it also manage all the routing of signals and method calls.

void ObjectManager::message_handle(Message& msg) {
    if (msg.is_signal(_interface_name, "InterfacesAdded")) {
        std::string path = msg.extract().get<std::string>();
        msg.extract_next();
        Holder options = msg.extract();

        proxy()->path_add(path, options);
        InterfacesAdded(path, options);
    } else if (msg.is_signal(_interface_name, "InterfacesRemoved")) {
        std::string path = msg.extract().get<std::string>();
        msg.extract_next();
        Holder options = msg.extract();

        proxy()->path_remove(path, options);
        InterfacesRemoved(path, options);
    } else if (msg.is_method_call(_interface_name, "GetManagedObjects")) {
        Holder result = proxy()->path_collect();

        Message reply = Message::create_method_return(msg);
        reply.append_argument(result, "a{oa{sa{sv}}}");
        _conn->send(reply);
    }
}
