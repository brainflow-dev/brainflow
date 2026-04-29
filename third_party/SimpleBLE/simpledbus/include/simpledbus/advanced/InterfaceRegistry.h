#pragma once

#include <memory>
#include <string>

#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Holder.h>
#include <simpledbus/base/Logging.h>
#include <simpledbus/advanced/Interface.h>
#include <simpledbus/advanced/Proxy.h>

namespace SimpleDBus {

using CreatorFunction = std::shared_ptr<Interface> (*)(std::shared_ptr<Connection>, std::shared_ptr<Proxy>);

class InterfaceRegistry {
  public:
    static InterfaceRegistry& getInstance() {
        static InterfaceRegistry instance;
        return instance;
    }

    template <typename T>
    void registerClass(const std::string& iface_name, CreatorFunction creator) {
        static_assert(std::is_base_of<Interface, T>::value, "T must inherit from Interface");
        creators[iface_name] = creator;
    }

    bool isRegistered(const std::string& iface_name) const { return creators.find(iface_name) != creators.end(); }

    // NOTES; We need a method inside Interfaces that will automatically retrieve the Interface name for the class.
    std::shared_ptr<Interface> create(const std::string& iface_name, std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy,
                                      const Holder& options) const {
        auto it = creators.find(iface_name);
        if (it != creators.end()) {
            auto iface = it->second(conn, proxy);
            iface->load(options);
            return iface;
        }
        return nullptr;
    }

  private:
    std::unordered_map<std::string, CreatorFunction> creators;
    InterfaceRegistry() = default;
};

template <typename T>
struct AutoRegisterInterface {
    AutoRegisterInterface(const std::string& iface_name, CreatorFunction creator) {
        static_assert(std::is_base_of<Interface, T>::value, "T must inherit from Interface");
        InterfaceRegistry::getInstance().registerClass<T>(iface_name, creator);
        LOG_DEBUG("Registered class with iface_name {}", iface_name);
    }
};

}  // namespace SimpleDBus
