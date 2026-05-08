#pragma once

#include <simpledbuslegacy/base/Connection.h>

#include <simpledbuslegacy/advanced/Interface.h>
#include <kvn/kvn_safe_callback.hpp>
#include <simpledbuslegacy/base/Path.h>

#include <memory>
#include <mutex>
#include <string>

namespace SimpleDBusLegacy {

class Interface;

class Proxy {
  public:
    Proxy(std::shared_ptr<Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Proxy();

    bool valid() const;
    void invalidate();
    std::string path() const;
    std::string bus_name() const;

    bool path_exists(const std::string& path);
    std::shared_ptr<Proxy> path_get(const std::string& path);

    bool interface_exists(const std::string& name);
    std::shared_ptr<Interface> interface_get(const std::string& name);

    const std::map<std::string, std::shared_ptr<Proxy>>& children();
    const std::map<std::string, std::shared_ptr<Interface>>& interfaces();

    virtual std::shared_ptr<Proxy> path_create(const std::string& path);

    // ----- PATH HANDLING -----
    void register_object_path();
    void unregister_object_path();

    // ----- INTROSPECTION -----
    // ! TODO: This should be moved to the Introspectable interface.
    std::string introspect();

    // ----- INTERFACE HANDLING -----
    // // ! We are making the assumption that the Properties interface is always available.
    // std::shared_ptr<Properties> properties() {
    //     return std::dynamic_pointer_cast<Properties>(interface_get("org.freedesktop.DBus.Properties"));
    // }

    size_t interfaces_count();
    bool interfaces_loaded();
    void interfaces_load(Holder managed_interfaces);
    void interfaces_reload(Holder managed_interfaces);
    void interfaces_unload(Holder removed_interfaces);

    // ----- CHILD HANDLING -----
    void path_add(const std::string& path, Holder managed_interfaces);
    bool path_remove(const std::string& path, Holder removed_interfaces);
    bool path_prune();
    Holder path_collect();

    // ----- MANUAL CHILD HANDLING -----
    // ! This function is used to manually add children to the proxy.
    void path_append_child(const std::string& path, std::shared_ptr<Proxy> child);
    void path_remove_child(const std::string& path);
    // ----- MESSAGE HANDLING -----
    void message_forward(Message& msg);
    void message_handle(Message& msg);

    // ----- CALLBACKS -----
    kvn::safe_callback<void(std::string)> on_child_created;
    kvn::safe_callback<void()> on_signal_received;

    // ----- TEMPLATE METHODS -----
    // ! This method returns a Proxy descendant object.
    template <typename T>
    std::vector<std::shared_ptr<T>> children_casted() {
        std::vector<std::shared_ptr<T>> result;
        std::scoped_lock lock(_child_access_mutex);
        for (auto& [path, child] : _children) {
            result.push_back(std::dynamic_pointer_cast<T>(child));
        }
        return result;
    }

    // ! This method returns a Proxy descendant object.
    template <typename T>
    std::vector<std::shared_ptr<T>> children_casted_with_prefix(const std::string& prefix) {
        std::vector<std::shared_ptr<T>> result;
        std::scoped_lock lock(_child_access_mutex);
        for (auto& [path, child] : _children) {
            const std::string next_child = SimpleDBusLegacy::Path::next_child_strip(_path, path);
            if (next_child.find(prefix) == 0) {
                result.push_back(std::dynamic_pointer_cast<T>(child));
            }
        }
        return result;
    }

  protected:
    bool _valid;
    bool _registered;
    std::string _path;
    std::string _bus_name;

    std::shared_ptr<Connection> _conn;

    std::map<std::string, std::shared_ptr<Interface>> _interfaces;
    std::map<std::string, std::shared_ptr<Proxy>> _children;

    std::recursive_mutex _interface_access_mutex;
    std::recursive_mutex _child_access_mutex;
};

}  // namespace SimpleDBusLegacy
