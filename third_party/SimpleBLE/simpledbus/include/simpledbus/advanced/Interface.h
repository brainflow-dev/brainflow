#pragma once

#include <simpledbus/base/Connection.h>

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include "kvn/kvn_safe_callback.hpp"

namespace SimpleDBus {

class Proxy;

class Interface;
class PropertyBase;
template <typename T>
class Property;
template <typename T>
class CustomProperty;

class Interface {
  public:
    class PropertyBase {
      public:
        PropertyBase(Interface& interface, const std::string& name) : _interface(interface), _name(name), _valid(false) {}

        virtual ~PropertyBase() = default;

        // Delete copy, allow move
        PropertyBase(const PropertyBase&) = delete;
        PropertyBase& operator=(const PropertyBase&) = delete;
        PropertyBase(PropertyBase&&) noexcept = delete;
        PropertyBase& operator=(PropertyBase&&) noexcept = delete;

        PropertyBase& refresh() {
            _interface.property_refresh(_name);
            return *this;
        }

        void emit() { _interface.property_emit(_name, get()); }

        Holder get() const {
            std::scoped_lock lock(_mutex);
            return _value;
        }

        PropertyBase& set(Holder value) {
            std::scoped_lock lock(_mutex);
            _value = value;
            _valid = true;
            notify_changed();
            return *this;
        }

        bool valid() {
            std::scoped_lock lock(_mutex);
            return _valid;
        }

        void invalidate() {
            std::scoped_lock lock(_mutex);
            _valid = false;
        }

        bool operator==(const Holder& other) const {
            std::scoped_lock lock(_mutex);
            return _value == other;
        }

        bool operator!=(const Holder& other) const { return !(*this == other); }

        virtual void notify_changed() {}

      protected:
        Interface& _interface;
        const std::string _name;
        mutable std::recursive_mutex _mutex;
        Holder _value;
        bool _valid;
    };

    template <typename T>
    class Property : public PropertyBase {
      public:
        Property(Interface& interface, const std::string& name) : PropertyBase(interface, name) {}

        virtual ~Property() { on_changed.unload(); }

        T operator()() const { return get(); }
        operator T() const { return get(); }

        // NOTE: This is a workaround to allow implicit conversion to std::string for ObjectPath and Signature types.
        template <typename U = T, typename = std::enable_if_t<!std::is_same_v<U, std::string> && std::is_convertible_v<U, std::string>>>
        operator std::string() const {
            return (std::string)get();
        }

        void operator()(const T& value) { set(value); }

        using PropertyBase::set;

        Property& refresh() {
            PropertyBase::refresh();
            return *this;
        }

        T get() const {
            std::scoped_lock lock(_mutex);
            return _value.template get<T>();
        }

        Property& set(T value) {
            std::scoped_lock lock(_mutex);
            _value = Holder::create<T>(value);
            _valid = true;
            return *this;
        }

        kvn::safe_callback<void(T)> on_changed;

        void notify_changed() override { on_changed(get()); }
    };

    Interface(std::shared_ptr<Connection> conn, std::shared_ptr<Proxy> proxy, const std::string& interface_name);

    virtual ~Interface() = default;

    // ----- LIFE CYCLE -----
    void load(Holder options);
    void unload();
    bool is_loaded() const;

    // ----- METHODS -----
    Message create_method_call(const std::string& method_name);

    // ----- PROPERTIES -----
    bool property_exists(const std::string& property_name);
    void property_refresh(const std::string& property_name);
    void property_emit(const std::string& property_name, Holder value);

    // ----- MESSAGES -----
    virtual void message_handle(Message& msg) {}

    // ----- HANDLES -----
    void handle_properties_changed(Holder changed_properties, Holder invalidated_properties);
    void handle_property_set(std::string property_name, Holder value);
    Holder handle_property_get(std::string property_name);
    Holder handle_property_get_all();

    template <typename T>
    Property<T>& property(const std::string& name) {
        std::unique_ptr<PropertyBase> property_ptr = std::make_unique<Property<T>>(*this, name);
        Property<T>& property = dynamic_cast<Property<T>&>(*property_ptr);
        property.set(T());
        _properties.emplace(name, std::move(property_ptr));
        return property;
    }

    template <typename T>
    Property<T>& property(const std::string& name, T default_value) {
        std::unique_ptr<PropertyBase> property_ptr = std::make_unique<Property<T>>(*this, name);
        Property<T>& property = dynamic_cast<Property<T>&>(*property_ptr);
        property.set(default_value);
        _properties.emplace(name, std::move(property_ptr));
        return property;
    }

  protected:
    std::atomic_bool _loaded{true};

    std::string _path;
    std::string _bus_name;
    std::string _interface_name;
    std::shared_ptr<Connection> _conn;
    std::weak_ptr<Proxy> _proxy;

    std::shared_ptr<Proxy> proxy() const;

    // IMPORTANT: Never erase from _property_bases during lifetime of Interface.
    // "Removal" of a property means: invalidate it via invalidate() or set _valid = false.
    // The entry must remain so that permanent references in derived classes stay valid.
    std::map<std::string, std::unique_ptr<PropertyBase>> _properties;
};

}  // namespace SimpleDBus
