#include <simpledbus/base/Logger.h>
#include <simpledbus/common/ObjectManager.h>

using namespace SimpleDBus;

ObjectManager::ObjectManager(Connection* conn, std::string service, std::string path)
    : _conn(conn), _service(service), _path(path), _interface("org.freedesktop.DBus.ObjectManager") {}

ObjectManager::~ObjectManager() {}

Holder ObjectManager::GetManagedObjects(bool use_callbacks) {
    Message query_msg = Message::create_method_call(_service, _path, _interface, "GetManagedObjects");
    Message reply_msg = _conn->send_with_reply_and_block(query_msg);
    Holder managed_objects = reply_msg.extract();
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

bool ObjectManager::process_received_signal(Message& message) {
    if (message.get_path() == _path) {
        if (message.is_signal(_interface, "InterfacesAdded")) {
            std::string path = message.extract().get_string();
            message.extract_next();
            Holder options = message.extract();
            if (InterfacesAdded) {
                InterfacesAdded(path, options);
            }
            return true;
        } else if (message.is_signal(_interface, "InterfacesRemoved")) {
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
