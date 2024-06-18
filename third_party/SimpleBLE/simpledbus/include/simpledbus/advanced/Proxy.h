#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/external/kvn_safe_callback.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace SimpleDBus {

class Proxy {
  public:
    Proxy(std::shared_ptr<Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Proxy();

    bool valid() const;
    std::string path() const;

    bool path_exists(const std::string& path);
    std::shared_ptr<Proxy> path_get(const std::string& path);

    bool interface_exists(const std::string& name);
    std::shared_ptr<Interface> interface_get(const std::string& name);

    const std::map<std::string, std::shared_ptr<Proxy>>& children();
    const std::map<std::string, std::shared_ptr<Interface>>& interfaces();

    virtual std::shared_ptr<Interface> interfaces_create(const std::string& name);
    virtual std::shared_ptr<Proxy> path_create(const std::string& path);

    // ----- INTROSPECTION -----
    std::string introspect();

    // ----- INTERFACE HANDLING -----
    size_t interfaces_count();
    bool interfaces_loaded();
    void interfaces_load(Holder managed_interfaces);
    void interfaces_reload(Holder managed_interfaces);
    void interfaces_unload(Holder removed_interfaces);

    // ----- CHILD HANDLING -----
    void path_add(const std::string& path, Holder managed_interfaces);
    bool path_remove(const std::string& path, Holder removed_interfaces);
    bool path_prune();
    void path_append_child(const std::string& path, std::shared_ptr<Proxy> child);

    // ----- MESSAGE HANDLING -----
    void message_forward(Message& msg);

    // ----- CALLBACKS -----
    kvn::safe_callback<void(std::string)> on_child_created;
    kvn::safe_callback<void(std::string)> on_child_signal_received;

    // ----- TEMPLATE METHODS -----
    template <typename T>
    std::vector<std::shared_ptr<T>> children_casted() {
        std::vector<std::shared_ptr<T>> result;
        std::scoped_lock lock(_child_access_mutex);
        for (auto& [path, child] : _children) {
            result.push_back(std::dynamic_pointer_cast<T>(child));
        }
        return result;
    }

  protected:
    bool _valid;
    std::string _path;
    std::string _bus_name;

    std::shared_ptr<Connection> _conn;

    std::map<std::string, std::shared_ptr<Interface>> _interfaces;
    std::map<std::string, std::shared_ptr<Proxy>> _children;

    std::recursive_mutex _interface_access_mutex;
    std::recursive_mutex _child_access_mutex;
};

}  // namespace SimpleDBus
