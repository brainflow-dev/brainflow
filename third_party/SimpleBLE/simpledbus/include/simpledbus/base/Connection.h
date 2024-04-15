#pragma once

#include <dbus/dbus.h>
#include <mutex>
#include "Message.h"

namespace SimpleDBus {

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
    Message pop_message();

    void send(Message& msg);
    Message send_with_reply_and_block(Message& msg);

    // ----- PROPERTIES -----
    std::string unique_name();

  private:
    bool _initialized = false;

    ::DBusBusType _dbus_bus_type;
    ::DBusConnection* _conn;

    std::recursive_mutex _mutex;
};

}  // namespace SimpleDBus
