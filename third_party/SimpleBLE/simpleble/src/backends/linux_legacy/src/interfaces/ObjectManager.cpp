#include <simpledbuslegacy/interfaces/ObjectManager.h>

using namespace SimpleDBusLegacy;

const AutoRegisterInterface<ObjectManager> ObjectManager::registry{
    "org.freedesktop.DBus.ObjectManager",
    // clang-format off
    [](std::shared_ptr<Connection> conn, const std::string& bus_name, const std::string& path, const Holder& options) -> std::shared_ptr<SimpleDBusLegacy::Interface> {
        return std::static_pointer_cast<SimpleDBusLegacy::Interface>(std::make_shared<ObjectManager>(conn, bus_name, path));
    }
    // clang-format on
};

ObjectManager::ObjectManager(std::shared_ptr<Connection> conn, std::string bus_name, std::string path)
    : Interface(conn, bus_name, path, "org.freedesktop.DBus.ObjectManager") {}

Holder ObjectManager::GetManagedObjects(bool use_callbacks) {
    Message query_msg = Message::create_method_call(_bus_name, _path, _interface_name, "GetManagedObjects");
    Message reply_msg = _conn->send_with_reply_and_block(query_msg);
    Holder managed_objects = reply_msg.extract();
    // TODO: Remove immediate callback support.
    if (use_callbacks) {
        auto managed_object = managed_objects.get_dict_object_path();
        for (auto& [path, options] : managed_object) {
            if (InterfacesAdded) {
                InterfacesAdded(path, options);
            }
        }
    }
    return managed_objects;
}

void ObjectManager::message_handle(Message& msg) {
    if (msg.is_signal(_interface_name, "InterfacesAdded")) {
        std::string path = msg.extract().get_string();
        msg.extract_next();
        Holder options = msg.extract();
        if (InterfacesAdded) {
            InterfacesAdded(path, options);
        }
    } else if (msg.is_signal(_interface_name, "InterfacesRemoved")) {
        std::string path = msg.extract().get_string();
        msg.extract_next();
        Holder options = msg.extract();
        if (InterfacesRemoved) {
            InterfacesRemoved(path, options);
        }
        // TODO: Make a call directly to the proxy to do this?

    } else if (msg.is_method_call(_interface_name, "GetManagedObjects")) {
        // TODO: Implement this.
        // SimpleDBus::Holder result = _proxy->path_collect();

        // SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);
        // reply.append_argument(result, "a{oa{sa{sv}}}");
        // _conn->send(reply);
    }
}
