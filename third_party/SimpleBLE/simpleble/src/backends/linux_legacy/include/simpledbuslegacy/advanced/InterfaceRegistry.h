#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>

#include <simpledbuslegacy/base/Connection.h>
#include <simpledbuslegacy/base/Holder.h>

namespace SimpleDBusLegacy {

class Interface;

using CreatorFunction = std::shared_ptr<Interface> (*)(std::shared_ptr<Connection>, const std::string&,
                                                       const std::string&, const Holder&);

class InterfaceRegistry {
  public:
    static InterfaceRegistry& getInstance() {
        static InterfaceRegistry instance;
        return instance;
    }

    template <typename T>
    void registerClass(const std::string& key, CreatorFunction creator) {
        static_assert(std::is_base_of<Interface, T>::value, "T must inherit from Interface");
        creators[key] = creator;
    }

    bool isRegistered(const std::string& key) const {
        return creators.find(key) != creators.end();
    }

    // NOTES; We need a method inside Interfaces that will automatically retrieve the Interface name for the class.
    std::shared_ptr<Interface> create(const std::string& key, std::shared_ptr<Connection> conn,
                                      const std::string& bus_name, const std::string& path,
                                      const Holder& options) const {
        auto it = creators.find(key);
        if (it != creators.end()) {
            auto iface = it->second(conn, bus_name, path, options);
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
    AutoRegisterInterface(const std::string& key, CreatorFunction creator) {
        static_assert(std::is_base_of<Interface, T>::value, "T must inherit from Interface");
        InterfaceRegistry::getInstance().registerClass<T>(key, creator);
    }
};

}  // namespace SimpleDBusLegacy
