#pragma once

#include <dbus/dbus.h>
#include <mutex>
#include <unordered_map>
#include <functional>
#include "Message.h"

namespace SimpleDBusLegacy {

class Message;

class Connection {
  public:
    Connection(::DBusBusType dbus_bus_type);
    ~Connection();

    void init();
    void uninit();
    bool is_initialized();

    void add_match(std::string rule);
    void remove_match(std::string rule);

    void read_write();
    void read_write_dispatch();
    Message pop_message();

    void send(Message& msg);
    Message send_with_reply_and_block(Message& msg);

    bool register_object_path(const std::string& path, std::function<void(Message&)> handler);
    bool unregister_object_path(const std::string& path);

    // ----- PROPERTIES -----
    std::string unique_name();

  private:
    bool _initialized = false;

    ::DBusBusType _dbus_bus_type;
    ::DBusConnection* _conn;

    std::recursive_mutex _mutex;

    static DBusHandlerResult static_message_handler(DBusConnection* connection, DBusMessage* message, void* user_data);
    std::unordered_map<std::string, std::function<void(Message&)>> _message_handlers;
};

}  // namespace SimpleDBusLegacy
