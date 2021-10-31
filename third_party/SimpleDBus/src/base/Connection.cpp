#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Logger.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace SimpleDBus;

Connection::Connection(DBusBusType dbus_bus_type) : _dbus_bus_type(dbus_bus_type) {}

Connection::~Connection() {
    // In order to prevent a crash on any third party environment
    // we need to flush the connection queue.
    SimpleDBus::Message message;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        read_write();
        message = pop_message();
    } while (message.is_valid());
    // ---------------------------------------------------------

    dbus_error_free(&_err);
    dbus_connection_unref(_conn);
}

void Connection::init() {
    dbus_threads_init_default();
    dbus_error_init(&_err);
    _conn = dbus_bus_get(_dbus_bus_type, &_err);
    if (dbus_error_is_set(&_err)) {
        LOG_F(ERROR, "Failed to get the DBus bus. (%s: %s)", _err.name, _err.message);
        dbus_error_free(&_err);
    }
}

void Connection::add_match(std::string rule) {
    dbus_bus_add_match(_conn, rule.c_str(), &_err);
    dbus_connection_flush(_conn);
    if (dbus_error_is_set(&_err)) {
        LOG_F(ERROR, "Failed to add match. (%s: %s)", _err.name, _err.message);
        dbus_error_free(&_err);
    }
}

void Connection::remove_match(std::string rule) {
    dbus_bus_remove_match(_conn, rule.c_str(), &_err);
    dbus_connection_flush(_conn);
    if (dbus_error_is_set(&_err)) {
        LOG_F(ERROR, "Failed to remove match. (%s: %s)", _err.name, _err.message);
        dbus_error_free(&_err);
    }
}

void Connection::read_write() {
    // Non blocking read of the next available message
    dbus_connection_read_write(_conn, 0);
}

Message Connection::pop_message() {
    DBusMessage* msg = dbus_connection_pop_message(_conn);
    if (msg == nullptr) {
        return Message();
    } else {
        return Message(msg);
    }
}

Message Connection::send_with_reply_and_block(Message& msg) {
    DBusMessage* msg_tmp = dbus_connection_send_with_reply_and_block(_conn, msg._msg, -1, &_err);

    if (dbus_error_is_set(&_err)) {
        LOG_F(WARN, "Message send failed. (%s: %s)", _err.name, _err.message);
        dbus_error_free(&_err);
        return Message();
    } else {
        return Message(msg_tmp);
    }
}
