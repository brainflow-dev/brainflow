#include <simpledbus/interfaces/ObjectManager.h>

using namespace SimpleDBus;

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
    }
}

bool ObjectManager::process_received_signal(Message& message) {
    if (message.get_path() == _path) {
        if (message.is_signal(_interface_name, "InterfacesAdded")) {
            std::string path = message.extract().get_string();
            message.extract_next();
            Holder options = message.extract();
            if (InterfacesAdded) {
                InterfacesAdded(path, options);
            }
            return true;
        } else if (message.is_signal(_interface_name, "InterfacesRemoved")) {
            std::string path = message.extract().get_string();
            message.extract_next();
            Holder options = message.extract();
            if (InterfacesRemoved) {
                InterfacesRemoved(path, options);
            }
            return true;
        }
    }
    return false;
}
