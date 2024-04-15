#include "simpledbus/advanced/Proxy.h"

#include <simpledbus/base/Exceptions.h>
#include <simpledbus/base/Path.h>
#include <algorithm>

using namespace SimpleDBus;

Proxy::Proxy(std::shared_ptr<Connection> conn, const std::string& bus_name, const std::string& path)
    : _conn(conn), _bus_name(bus_name), _path(path), _valid(true) {}

Proxy::~Proxy() {
    on_child_created.unload();
    on_child_signal_received.unload();
}

std::shared_ptr<Interface> Proxy::interfaces_create(const std::string& name) {
    return std::make_unique<Interface>(_conn, _bus_name, _path, name);
}

std::shared_ptr<Proxy> Proxy::path_create(const std::string& path) {
    return std::make_shared<Proxy>(_conn, _bus_name, path);
}

bool Proxy::valid() const { return _valid; }

std::string Proxy::path() const { return _path; }

const std::map<std::string, std::shared_ptr<Proxy>>& Proxy::children() { return _children; }

const std::map<std::string, std::shared_ptr<Interface>>& Proxy::interfaces() { return _interfaces; }

// ----- INTROSPECTION -----
std::string Proxy::introspect() {
    auto query_msg = Message::create_method_call(_bus_name, _path, "org.freedesktop.DBus.Introspectable", "Introspect");
    auto reply_msg = _conn->send_with_reply_and_block(query_msg);
    return reply_msg.extract().get_string();
}

// ----- INTERFACE HANDLING -----

bool Proxy::interface_exists(const std::string& name) {
    std::scoped_lock lock(_interface_access_mutex);
    return _interfaces.find(name) != _interfaces.end();
}

std::shared_ptr<Interface> Proxy::interface_get(const std::string& name) {
    std::scoped_lock lock(_interface_access_mutex);
    if (!interface_exists(name)) {
        throw Exception::InterfaceNotFoundException(_path, name);
    }
    return _interfaces[name];
}

size_t Proxy::interfaces_count() {
    size_t count = 0;
    std::scoped_lock lock(_interface_access_mutex);
    for (auto& [iface_name, interface] : _interfaces) {
        if (interface->is_loaded()) {
            count++;
        }
    }
    return count;
}

void Proxy::interfaces_load(Holder managed_interfaces) {
    auto managed_interface = managed_interfaces.get_dict_string();

    std::scoped_lock lock(_interface_access_mutex);
    for (auto& [iface_name, options] : managed_interface) {
        // If the interface has not been loaded, load it
        if (!interface_exists(iface_name)) {
            _interfaces.emplace(std::make_pair(iface_name, interfaces_create(iface_name)));
        }

        _interfaces[iface_name]->load(options);
    }
}

void Proxy::interfaces_reload(Holder managed_interfaces) {
    std::scoped_lock lock(_interface_access_mutex);
    for (auto& [iface_name, interface] : _interfaces) {
        interface->unload();
    }

    interfaces_load(managed_interfaces);
}

void Proxy::interfaces_unload(SimpleDBus::Holder removed_interfaces) {
    std::scoped_lock lock(_interface_access_mutex);
    for (auto& option : removed_interfaces.get_array()) {
        std::string iface_name = option.get_string();
        if (interface_exists(iface_name)) {
            _interfaces[iface_name]->unload();
        }
    }
}

bool Proxy::interfaces_loaded() {
    std::scoped_lock lock(_interface_access_mutex);
    for (auto& [iface_name, interface] : _interfaces) {
        if (interface->is_loaded()) {
            return true;
        }
    }
    return false;
}

// ----- CHILD HANDLING -----

bool Proxy::path_exists(const std::string& path) {
    std::scoped_lock lock(_child_access_mutex);
    return _children.find(path) != _children.end();
}

std::shared_ptr<Proxy> Proxy::path_get(const std::string& path) {
    std::scoped_lock lock(_child_access_mutex);
    if (!path_exists(path)) {
        throw Exception::PathNotFoundException(_path, path);
    }
    return _children[path];
}

void Proxy::path_add(const std::string& path, SimpleDBus::Holder managed_interfaces) {
    // If the path is not a child of the current path, then we can't add it.
    if (!Path::is_descendant(_path, path)) {
        // TODO: Should an exception be thrown here?
        return;
    }

    // If the path is already in the map, perform a reload of all interfaces.
    if (path_exists(path)) {
        path_get(path)->interfaces_load(managed_interfaces);
        return;
    }

    // As children will be extensively accessed, we need to lock the child access mutex.
    std::scoped_lock lock(_child_access_mutex);

    if (Path::is_child(_path, path)) {
        // If the path is a direct child of the proxy path, create a new proxy for it.
        std::shared_ptr<Proxy> child = path_create(path);
        child->interfaces_load(managed_interfaces);
        _children.emplace(std::make_pair(path, child));
        on_child_created(path);
    } else {
        // If the new path is for a descendant of the current proxy, check if there is a child proxy for it.
        auto child_result = std::find_if(
            _children.begin(), _children.end(),
            [path](const std::pair<std::string, std::shared_ptr<Proxy>>& child_data) -> bool {
                return Path::is_descendant(child_data.first, path);
            });

        if (child_result != _children.end()) {
            // If there is a child proxy for the new path, forward it to that child proxy.
            child_result->second->path_add(path, managed_interfaces);
        } else {
            // If there is no child proxy for the new path, create the child and forward the path to it.
            // This path will be taken if an empty proxy object needs to be created for an intermediate path.
            std::string child_path = Path::next_child(_path, path);
            std::shared_ptr<Proxy> child = path_create(child_path);
            _children.emplace(std::make_pair(child_path, child));
            child->path_add(path, managed_interfaces);
            on_child_created(child_path);
        }
    }
}

bool Proxy::path_remove(const std::string& path, SimpleDBus::Holder options) {
    // `options` contains an array of strings of the interfaces that need to be removed.

    if (path == _path) {
        _valid = false;
        interfaces_unload(options);
        return path_prune();
    }

    // If the path is not the current path nor a descendant, then there's nothing to do
    if (!Path::is_descendant(_path, path)) {
        return false;
    }

    // As children will be extensively accessed, we need to lock the child access mutex.
    std::scoped_lock lock(_child_access_mutex);

    // If the path is a direct child of the proxy path, forward the request to the child proxy.
    std::string child_path = Path::next_child(_path, path);
    if (path_exists(child_path)) {
        bool must_erase = _children.at(child_path)->path_remove(path, options);

        // if the child proxy is no longer needed and there is only one active instance of the child proxy,
        // then remove it.
        if (must_erase && _children.at(child_path).use_count() == 1) {
            _children.erase(child_path);
        }
    }

    return false;
}

bool Proxy::path_prune() {
    // As children will be extensively accessed, we need to lock the child access mutex.
    std::scoped_lock lock(_child_access_mutex);

    // For each child proxy, check if it can be pruned.
    std::vector<std::string> to_remove;
    for (auto& [child_path, child] : _children) {
        if (child->path_prune() && _children.at(child_path).use_count() == 1) {
            to_remove.push_back(child_path);
        }
    }
    for (auto& child_path : to_remove) {
        _children.erase(child_path);
    }

    // For self to be pruned, the following conditions must be met:
    // 1. The proxy has no children
    // 2. The proxy has no interfaces or all interfaces are disabled.
    if (_children.empty() && !interfaces_loaded()) {
        return true;
    }

    return false;
}

void Proxy::path_append_child(const std::string& path, std::shared_ptr<Proxy> child) {
    // If the provided path is not a child of the current path, return silently.
    if (!Path::is_child(_path, path)) {
        // TODO: Should an exception be thrown here?
        return;
    }

    // As children will be extensively accessed, we need to lock the child access mutex.
    std::scoped_lock lock(_child_access_mutex);
    _children.emplace(std::make_pair(path, child));
}

// ----- MESSAGE HANDLING -----
void Proxy::message_forward(Message& msg) {
    // If the message is for the current proxy, then forward it to the message handler.
    if (msg.get_path() == _path) {
        // If the message is involves a property change, forward it to the correct interface.
        if (msg.is_signal("org.freedesktop.DBus.Properties", "PropertiesChanged")) {
            Holder interface_h = msg.extract();
            std::string iface_name = interface_h.get_string();
            msg.extract_next();
            Holder changed_properties = msg.extract();
            msg.extract_next();
            Holder invalidated_properties = msg.extract();

            // If the interface is not loaded, then ignore the message.
            if (!interface_exists(iface_name)) {
                return;
            }

            interface_get(iface_name)->signal_property_changed(changed_properties, invalidated_properties);

        } else if (interface_exists(msg.get_interface())) {
            interface_get(msg.get_interface())->message_handle(msg);
        }

        return;
    }

    // If the message is for a child proxy or a descendant, forward it to that child proxy.
    for (auto& [child_path, child] : _children) {
        if (child_path == msg.get_path()) {
            child->message_forward(msg);

            if (msg.get_type() == Message::Type::SIGNAL) {
                on_child_signal_received(child_path);
            }

            return;
        } else if (Path::is_descendant(child_path, msg.get_path())) {
            child->message_forward(msg);
            return;
        }
    }
}
