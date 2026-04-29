#include <simpledbus/Config.h>
#include <simpledbus/base/Connection.h>
#include <simpledbus/base/Exceptions.h>
#include <simpledbus/base/Logging.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

using namespace SimpleDBus;

Connection::Connection(DBusBusType dbus_bus_type) : _dbus_bus_type(dbus_bus_type) {}

Connection::~Connection() {
    if (_initialized) {
        uninit();
    }
}

void Connection::init() {
    if (_initialized) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    ::DBusError err;
    dbus_error_init(&err);

    dbus_threads_init_default();
    _conn = dbus_bus_get(_dbus_bus_type, &err);
    if (dbus_error_is_set(&err)) {
        std::string err_name = err.name;
        std::string err_message = err.message;
        dbus_error_free(&err);
        throw Exception::DBusException(err_name, err_message);
    }
    _initialized = true;
}

void Connection::uninit() {
    if (!_initialized) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    // In order to prevent a crash on any third party environment
    // we need to flush the connection queue.
    SimpleDBus::Message message;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        read_write_dispatch();
    } while (message.is_valid());

    dbus_connection_unref(_conn);
    _initialized = false;
}

bool Connection::is_initialized() { return _initialized; }

void Connection::add_match(std::string rule) {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    ::DBusError err;
    dbus_error_init(&err);

    dbus_bus_add_match(_conn, rule.c_str(), &err);
    dbus_connection_flush(_conn);
    if (dbus_error_is_set(&err)) {
        std::string err_name = err.name;
        std::string err_message = err.message;
        dbus_error_free(&err);
        throw Exception::DBusException(err_name, err_message);
    }
}

void Connection::remove_match(std::string rule) {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    ::DBusError err;
    dbus_error_init(&err);

    dbus_bus_remove_match(_conn, rule.c_str(), &err);
    dbus_connection_flush(_conn);
    if (dbus_error_is_set(&err)) {
        std::string err_name = err.name;
        std::string err_message = err.message;
        dbus_error_free(&err);
        throw Exception::DBusException(err_name, err_message);
    }
}

void Connection::read_write() {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    // Non blocking read of the next available message
    dbus_connection_read_write(_conn, 0);
}

void Connection::read_write_dispatch() {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    // Non-blocking read of the next available message
    dbus_connection_read_write(_conn, 0);

    // Dispatch incoming messages
    while (dbus_connection_dispatch(_conn) == DBUS_DISPATCH_DATA_REMAINS) {
    }
}

Message Connection::pop_message() {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    // std::lock_guard<std::recursive_mutex> lock(_mutex);

    DBusMessage* msg = dbus_connection_pop_message(_conn);
    return Message::from_acquired(msg);
}

void Connection::send(Message& msg) {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    // std::lock_guard<std::recursive_mutex> lock(_mutex);

    uint32_t msg_serial = 0;
    dbus_connection_send(_conn, msg, &msg_serial);
}

Message Connection::send_with_reply(Message& msg) {
    DBusPendingCall* pending = nullptr;
    dbus_connection_send_with_reply(_conn, msg, &pending, -1);

    if (!pending) {
        throw std::runtime_error("Failed to queue D-Bus message (Out of memory?)");
    }

    AsyncContext ctx;
    dbus_pending_call_set_notify(pending, static_reply_handler, &ctx, nullptr);

    bool timed_out = false;
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        if (!ctx.cv.wait_for(lock, Config::Connection::send_with_reply_timeout, [&ctx] { return ctx.completed; })) {
            timed_out = true;
        }
    }

    if (timed_out) {
        // Essential: cancel the pending call so the callback doesn't fire
        // later and access the 'ctx' which is about to be destroyed.
        dbus_pending_call_cancel(pending);
        dbus_pending_call_unref(pending);
        throw std::runtime_error("D-Bus call timed out");
    }

    // We have a result; the pending call handle is no longer needed.
    dbus_pending_call_unref(pending);

    DBusMessage* reply = ctx.reply;
    if (!reply) {
        throw std::runtime_error("Received null reply from D-Bus");
    }

    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        const char* err_name = dbus_message_get_error_name(reply);
        const char* err_text = "No error detail provided";

        // Try to extract the error string argument if it exists
        dbus_message_get_args(reply, nullptr, DBUS_TYPE_STRING, &err_text, DBUS_TYPE_INVALID);
        dbus_message_unref(reply);
        throw Exception::SendFailed(err_name, err_text, msg.to_string());
    }

    return Message::from_acquired(reply);
}

Message Connection::send_with_reply_and_block(Message& msg) {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    ::DBusError err;
    dbus_error_init(&err);
    DBusMessage* msg_tmp = dbus_connection_send_with_reply_and_block(_conn, msg, -1, &err);

    if (dbus_error_is_set(&err)) {
        std::string err_name = err.name;
        std::string err_message = err.message;
        dbus_error_free(&err);
        throw Exception::SendFailed(err_name, err_message, msg.to_string());
    }

    return Message::from_acquired(msg_tmp);
}

std::string Connection::unique_name() {
    if (!_initialized) {
        throw Exception::NotInitialized();
    }

    return std::string(dbus_bus_get_unique_name(_conn));
}

bool Connection::register_object_path(const std::string& path, std::function<void(Message&)> handler) {
    if (!_initialized) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);
    if (_message_handlers.find(path) == _message_handlers.end()) {
        DBusObjectPathVTable vtable = {0};
        vtable.message_function = &Connection::static_message_handler;
        dbus_connection_register_object_path(_conn, path.c_str(), &vtable, this);
        _message_handlers[path] = std::move(handler);
    }

    return true;
}

bool Connection::unregister_object_path(const std::string& path) {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    auto it = _message_handlers.find(path);
    if (it != _message_handlers.end()) {
        dbus_connection_unregister_object_path(_conn, path.c_str());
        _message_handlers.erase(it);
    }

    return true;
}

DBusHandlerResult Connection::static_message_handler(DBusConnection* connection, DBusMessage* message,
                                                     void* user_data) {
    Connection* conn = static_cast<Connection*>(user_data);
    Message msg = Message::from_retained(message);
    std::string path = msg.get_path();

    std::lock_guard<std::recursive_mutex> lock(conn->_mutex);
    auto it = conn->_message_handlers.find(path);
    if (it != conn->_message_handlers.end()) {
        it->second(msg);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

void Connection::static_reply_handler(DBusPendingCall* pending, void* user_data) {
    auto* ctx = static_cast<AsyncContext*>(user_data);

    std::lock_guard<std::mutex> lock(ctx->mtx);

    // Steal the reply from the pending call object
    ctx->reply = dbus_pending_call_steal_reply(pending);
    ctx->completed = true;

    // Wake the send_with_reply thread
    ctx->cv.notify_one();
}