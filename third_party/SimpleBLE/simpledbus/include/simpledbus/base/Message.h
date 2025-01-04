#pragma once

#include <dbus/dbus.h>
#include <atomic>
#include <string>
#include <vector>
#include "Holder.h"

namespace SimpleDBus {

class Message {
  public:
    enum class Type {
        INVALID = DBUS_MESSAGE_TYPE_INVALID,
        METHOD_CALL = DBUS_MESSAGE_TYPE_METHOD_CALL,
        METHOD_RETURN = DBUS_MESSAGE_TYPE_METHOD_RETURN,
        ERROR = DBUS_MESSAGE_TYPE_ERROR,
        SIGNAL = DBUS_MESSAGE_TYPE_SIGNAL,
    };

    static constexpr int INVALID_UNIQUE_ID = -1;

    Message();
    Message(Message&& other) noexcept;
    Message(const Message& other);
    Message& operator=(Message&& other) noexcept;
    Message& operator=(const Message& other);
    ~Message();

    operator DBusMessage*() const;

    bool is_valid() const;
    void append_argument(const Holder& argument, const std::string& signature);
    Holder extract();
    void extract_reset();
    bool extract_has_next();
    void extract_next();
    std::string to_string(bool append_arguments = false) const;

    uint32_t get_ref_count() const;
    int32_t get_unique_id() const;
    uint32_t get_serial() const;
    std::string get_signature();
    std::string get_interface() const;
    std::string get_path() const;
    std::string get_member() const;
    Type get_type() const;

    bool is_signal(const std::string& interface, const std::string& signal_name) const;
    bool is_method_call(const std::string& interface, const std::string& method) const;

    static Message from_retained(DBusMessage* msg);
    static Message from_acquired(DBusMessage* msg);
    static Message create_method_call(const std::string& bus_name, const std::string& path,
                                      const std::string& interface, const std::string& method);
    static Message create_method_return(const Message& msg);
    static Message create_error(const Message& msg, const std::string& error_name, const std::string& error_message);
    static Message create_signal(const std::string& path, const std::string& interface, const std::string& signal);

  private:
    static std::atomic_int32_t _creation_counter;

    int _indent = 0;
    int32_t _unique_id = INVALID_UNIQUE_ID;
    DBusMessageIter _iter;
    bool _iter_initialized = false;
    bool _is_extracted = false;
    Holder _extracted;
    DBusMessage* _msg = nullptr;
    std::vector<Holder> _arguments;

    Holder _extract_bytearray(DBusMessageIter* iter);
    Holder _extract_array(DBusMessageIter* iter);
    Holder _extract_dict(DBusMessageIter* iter);
    Holder _extract_generic(DBusMessageIter* iter);

    /**
     * @brief Append argument to the DBus message iterator.
     * @param iter      DBus message iterator.
     * @param argument  Argument to append.
     * @param signature Signature of the argument.
     */
    void _append_argument(DBusMessageIter* iter, const Holder& argument, const std::string& signature);

    void _invalidate();
    void _safe_delete();
};

}  // namespace SimpleDBus