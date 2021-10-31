#pragma once

#include <dbus/dbus.h>
#include "Message.h"

namespace SimpleDBus {

class Message;

class Connection {
  private:
    ::DBusBusType _dbus_bus_type;
    ::DBusConnection* _conn;
    ::DBusError _err;

  public:
    Connection(::DBusBusType dbus_bus_type);
    ~Connection();

    void init();

    void add_match(std::string rule);
    void remove_match(std::string rule);

    void read_write();
    Message pop_message();

    Message send_with_reply_and_block(Message& msg);
};

}  // namespace SimpleDBus
