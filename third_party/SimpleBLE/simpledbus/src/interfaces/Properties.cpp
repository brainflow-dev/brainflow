#include <simpledbus/advanced/Proxy.h>
#include <simpledbus/interfaces/Properties.h>

using namespace SimpleDBus;
using namespace SimpleDBus::Interfaces;

const AutoRegisterInterface<Properties> Properties::registry{
    "org.freedesktop.DBus.Properties",
    // clang-format off
    [](std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy) -> std::shared_ptr<Interface> {
        return std::make_shared<Properties>(conn, proxy);
    }
    // clang-format on
};

Properties::Properties(std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy)
    : Interface(conn, proxy, "org.freedesktop.DBus.Properties") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
Properties::~Properties() = default;

Holder Properties::Get(const std::string& interface_name, const std::string& property_name) {
    Message query_msg = Message::create_method_call(_bus_name, _path, "org.freedesktop.DBus.Properties", "Get");

    Holder h_interface = Holder::create<std::string>(interface_name);
    query_msg.append_argument(h_interface, "s");

    Holder h_name = Holder::create<std::string>(property_name);
    query_msg.append_argument(h_name, "s");

    Message reply_msg = _conn->send_with_reply_and_block(query_msg);
    Holder result = reply_msg.extract();
    return result;
}

Holder Properties::GetAll(const std::string& interface_name) {
    Message query_msg = Message::create_method_call(_bus_name, _path, "org.freedesktop.DBus.Properties", "GetAll");

    Holder h_interface = Holder::create<std::string>(interface_name);
    query_msg.append_argument(h_interface, "s");

    Message reply_msg = _conn->send_with_reply_and_block(query_msg);
    Holder result = reply_msg.extract();
    return result;
}

void Properties::Set(const std::string& interface_name, const std::string& property_name, const Holder& value) {
    Message query_msg = Message::create_method_call(_bus_name, _path, "org.freedesktop.DBus.Properties", "Set");

    Holder h_interface = Holder::create<std::string>(interface_name);
    query_msg.append_argument(h_interface, "s");

    Holder h_name = Holder::create<std::string>(property_name);
    query_msg.append_argument(h_name, "s");

    query_msg.append_argument(value, "v");

    _conn->send_with_reply_and_block(query_msg);
}

void Properties::PropertiesChanged(const std::string& interface_name,
                                   const std::map<std::string, Holder>& changed_properties) {
    std::vector<std::string> invalidated_properties;
    PropertiesChanged(interface_name, changed_properties, invalidated_properties);
}

void Properties::PropertiesChanged(const std::string& interface_name,
                                   const std::vector<std::string>& invalidated_properties) {
    std::map<std::string, Holder> changed_properties;
    PropertiesChanged(interface_name, changed_properties, invalidated_properties);
}

void Properties::PropertiesChanged(const std::string& interface_name,
                                   const std::map<std::string, Holder>& changed_properties,
                                   const std::vector<std::string>& invalidated_properties) {
    Message signal_msg = Message::create_signal(_path, "org.freedesktop.DBus.Properties", "PropertiesChanged");

    Holder interface_h = Holder::create<std::string>(interface_name);
    signal_msg.append_argument(interface_h, "s");

    Holder changed_properties_h = Holder::create<std::map<std::string, Holder>>();
    for (const auto& [key, value] : changed_properties) {
        changed_properties_h.dict_append(Holder::STRING, key, value);
    }
    signal_msg.append_argument(changed_properties_h, "a{sv}");

    Holder invalidated_properties_h = Holder::create<std::vector<Holder>>();
    for (const auto& property : invalidated_properties) {
        invalidated_properties_h.array_append(Holder::create<std::string>(property));
    }
    signal_msg.append_argument(invalidated_properties_h, "as");
    _conn->send(signal_msg);
}

void Properties::message_handle(Message& msg) {
    if (msg.is_method_call(_interface_name, "GetAll")) {
        Holder interface_h = msg.extract();
        std::string iface_name = interface_h.get<std::string>();

        std::shared_ptr<Interface> interface = proxy()->interface_get(iface_name);
        Holder properties = interface->handle_property_get_all();

        Message reply = Message::create_method_return(msg);
        reply.append_argument(properties, "a{sv}");
        _conn->send(reply);

    } else if (msg.is_method_call(_interface_name, "Get")) {
        Holder interface_h = msg.extract();
        std::string iface_name = interface_h.get<std::string>();
        msg.extract_next();

        Holder property_h = msg.extract();
        std::string property_name = property_h.get<std::string>();

        std::shared_ptr<Interface> interface = proxy()->interface_get(iface_name);

        bool property_exists = interface->property_exists(property_name);

        if (property_exists) {
            Holder property_value = interface->handle_property_get(property_name);
            Message reply = Message::create_method_return(msg);
            reply.append_argument(property_value, "v");
            _conn->send(reply);
        } else {
            Message reply = Message::create_error(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Property not found");
            _conn->send(reply);
        }

    } else if (msg.is_method_call(_interface_name, "Set")) {
        Holder interface_h = msg.extract();
        std::string iface_name = interface_h.get<std::string>();
        msg.extract_next();

        Holder property_h = msg.extract();
        std::string property_name = property_h.get<std::string>();
        msg.extract_next();

        Holder value_h = msg.extract();

        std::shared_ptr<Interface> interface = proxy()->interface_get(iface_name);
        interface->handle_property_set(property_name, value_h);

        Message reply = Message::create_method_return(msg);
        _conn->send(reply);

    } else if (msg.is_signal(_interface_name, "PropertiesChanged")) {
        Holder interface_h = msg.extract();
        std::string iface_name = interface_h.get<std::string>();
        msg.extract_next();
        Holder changed_properties = msg.extract();
        msg.extract_next();
        Holder invalidated_properties = msg.extract();

        // If the interface is not loaded, then ignore the message.
        if (!proxy()->interface_exists(iface_name)) {
            return;
        }

        proxy()->interface_get(iface_name)->handle_properties_changed(changed_properties, invalidated_properties);
    }
}