#include <simpledbus/base/Message.h>

#include <sstream>

using namespace SimpleDBus;

#define MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents)                                   \
    for (auto& [key, value] : dict_contents) {                                                \
        DBusMessageIter entry_iter;                                                           \
        dbus_message_iter_open_container(&sub_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter); \
        dbus_message_iter_append_basic(&entry_iter, key_sig, &key);                           \
        _append_argument(&entry_iter, value, value_sig);                                      \
        dbus_message_iter_close_container(&sub_iter, &entry_iter);                            \
    }

#define MESSAGE_DICT_APPEND_KEY_STR(key_sig, dict_contents)                                   \
    for (auto& [key, value] : dict_contents) {                                                \
        const char* p_value = key.c_str();                                                    \
        DBusMessageIter entry_iter;                                                           \
        dbus_message_iter_open_container(&sub_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter); \
        dbus_message_iter_append_basic(&entry_iter, key_sig, &p_value);                       \
        _append_argument(&entry_iter, value, value_sig);                                      \
        dbus_message_iter_close_container(&sub_iter, &entry_iter);                            \
    }

std::atomic_int32_t Message::_creation_counter = 0;

Message::Message() {}

Message::~Message() {
    if (is_valid()) {
        _safe_delete();
    }
}

Message::Message(Message&& other) noexcept
    : _indent(other._indent),
      _unique_id(other._unique_id),
      _iter(std::move(other._iter)),
      _iter_initialized(other._iter_initialized),
      _is_extracted(other._is_extracted),
      _extracted(std::move(other._extracted)),
      _msg(other._msg),
      _arguments(std::move(other._arguments)) {
    // Move constructor: Transfer ownership of resources from 'other' to this object.
    // After the move, 'other' will be left in a valid but unspecified state.

    // Invalidate the moved-from object
    other._invalidate();
}

Message::Message(const Message& other)
    : _indent(other._indent),
      _unique_id(_creation_counter++),
      _is_extracted(other._is_extracted),
      _extracted(other._extracted),
      _arguments(other._arguments) {
    // Copy constructor: Create a new Message as a deep copy of 'other'.
    // If 'other' is valid, all its contents are duplicated.

    if (other._msg) {
        _msg = dbus_message_copy(other._msg);
        if (_msg == nullptr) {
            throw std::runtime_error("Failed to copy DBusMessage in copy constructor");
        }
    } else {
        _msg = nullptr;
    }
}

Message& Message::operator=(Message&& other) noexcept {
    // Move assignment operator: Transfer ownership of resources from 'other' to this object.
    // Any existing resources in this object are safely deleted before the transfer.
    // After the move, 'other' will be left in a valid but unspecified state.

    if (this != &other) {
        _safe_delete();  // Clean up existing resources

        // Transfer ownership
        _indent = other._indent;
        _unique_id = other._unique_id;
        _iter = other._iter;
        _iter_initialized = other._iter_initialized;
        _is_extracted = other._is_extracted;
        _extracted = std::move(other._extracted);
        _msg = other._msg;
        _arguments = std::move(other._arguments);

        // Invalidate the moved-from object
        other._invalidate();
    }
    return *this;
}

Message& Message::operator=(const Message& other) {
    // Copy assignment operator: Replace the contents of this Message with a deep copy of 'other'.
    // Any existing resources in this object are safely deleted before the copy.
    // If 'other' is valid, all its contents are duplicated.

    if (this != &other) {
        _safe_delete();  // Clean up existing resources

        _indent = other._indent;
        _unique_id = _creation_counter++;
        _is_extracted = other._is_extracted;
        _extracted = other._extracted;
        _arguments = other._arguments;

        if (other._msg) {
            _msg = dbus_message_copy(other._msg);
            if (_msg == nullptr) {
                throw std::runtime_error("Failed to copy DBusMessage in copy assignment");
            }
        } else {
            _msg = nullptr;
        }
    }
    return *this;
}

Message::operator DBusMessage*() const { return _msg; }

bool Message::is_valid() const { return _msg != nullptr; }

void Message::_append_argument(DBusMessageIter* iter, const Holder& argument, const std::string& signature) {
    switch (signature[0]) {
        case DBUS_TYPE_BYTE: {
            uint8_t value = argument.get_byte();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &value);
            break;
        }
        case DBUS_TYPE_BOOLEAN: {
            uint32_t value = static_cast<uint32_t>(argument.get_boolean());
            dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &value);
            break;
        }
        case DBUS_TYPE_INT16: {
            int16_t value = argument.get_int16();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16, &value);
            break;
        }
        case DBUS_TYPE_UINT16: {
            uint16_t value = argument.get_uint16();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &value);
            break;
        }
        case DBUS_TYPE_INT32: {
            int32_t value = argument.get_int32();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &value);
            break;
        }
        case DBUS_TYPE_UINT32: {
            uint32_t value = argument.get_uint32();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &value);
            break;
        }
        case DBUS_TYPE_INT64: {
            int64_t value = argument.get_int64();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_INT64, &value);
            break;
        }
        case DBUS_TYPE_UINT64: {
            uint64_t value = argument.get_uint64();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &value);
            break;
        }
        case DBUS_TYPE_DOUBLE: {
            double value = argument.get_double();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_DOUBLE, &value);
            break;
        }
        case DBUS_TYPE_STRING: {
            std::string value = argument.get_string();
            const char* p_value = value.c_str();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &p_value);
            break;
        }
        case DBUS_TYPE_OBJECT_PATH: {
            std::string value = argument.get_object_path();
            const char* p_value = value.c_str();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &p_value);
            break;
        }
        case DBUS_TYPE_SIGNATURE: {
            std::string value = argument.get_signature();
            const char* p_value = value.c_str();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_SIGNATURE, &p_value);
            break;
        }
        case DBUS_TYPE_VARIANT: {
            DBusMessageIter sub_iter;
            std::string signature = argument.signature();
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, signature.c_str(), &sub_iter);
            _append_argument(&sub_iter, argument, signature);
            dbus_message_iter_close_container(iter, &sub_iter);
            break;
        }
        case DBUS_TYPE_ARRAY: {
            auto sig_next = signature.substr(1);
            DBusMessageIter sub_iter;
            dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, sig_next.c_str(), &sub_iter);
            if (sig_next[0] != DBUS_DICT_ENTRY_BEGIN_CHAR) {
                auto array_contents = argument.get_array();
                for (auto elem : array_contents) {
                    _append_argument(&sub_iter, elem, sig_next);
                }
            } else {
                sig_next = sig_next.substr(1, sig_next.length() - 2);
                auto key_sig = sig_next[0];
                auto value_sig = sig_next.substr(1);

                switch (key_sig) {
                    case DBUS_TYPE_BYTE: {
                        auto dict_contents = argument.get_dict_uint8();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_INT16: {
                        auto dict_contents = argument.get_dict_int16();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_UINT16: {
                        auto dict_contents = argument.get_dict_uint16();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_INT32: {
                        auto dict_contents = argument.get_dict_int32();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_UINT32: {
                        auto dict_contents = argument.get_dict_uint32();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_INT64: {
                        auto dict_contents = argument.get_dict_int64();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_UINT64: {
                        auto dict_contents = argument.get_dict_uint64();
                        MESSAGE_DICT_APPEND_KEY_NUM(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_STRING: {
                        auto dict_contents = argument.get_dict_string();
                        MESSAGE_DICT_APPEND_KEY_STR(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_OBJECT_PATH: {
                        auto dict_contents = argument.get_dict_object_path();
                        MESSAGE_DICT_APPEND_KEY_STR(key_sig, dict_contents);
                        break;
                    }
                    case DBUS_TYPE_SIGNATURE: {
                        auto dict_contents = argument.get_dict_signature();
                        MESSAGE_DICT_APPEND_KEY_STR(key_sig, dict_contents);
                        break;
                    }
                }
            }
            dbus_message_iter_close_container(iter, &sub_iter);
        }
    }
}

void Message::append_argument(const Holder& argument, const std::string& signature) {
    dbus_message_iter_init_append(_msg, &_iter);
    _append_argument(&_iter, argument, signature);
    _arguments.push_back(argument);
}

uint32_t Message::get_ref_count() const {
    // NOTE: This is a hack based on the DBusMessage structure documentation that says that
    // the first 4 bytes of the DBusMessage structure is the ref count.
    // https://dbus.freedesktop.org/doc/api/html/structDBusMessage.html#a324c5377e0be18dd84ac519ab2d23f0d
    if (is_valid()) {
        return *reinterpret_cast<uint32_t*>(this->_msg);
    }
    return 0;
}

int32_t Message::get_unique_id() const { return _unique_id; }

uint32_t Message::get_serial() const {
    if (is_valid()) {
        return dbus_message_get_serial(_msg);
    }
    return 0;
}

std::string Message::get_signature() {
    if (is_valid() && _iter_initialized) {
        return dbus_message_iter_get_signature(&_iter);
    }
    return "";
}

Message::Type Message::get_type() const {
    if (is_valid()) {
        return static_cast<Message::Type>(dbus_message_get_type(_msg));
    }
    return Message::Type::INVALID;
}

std::string Message::get_path() const {
    if (is_valid() && (get_type() == Message::Type::SIGNAL || get_type() == Message::Type::METHOD_CALL)) {
        return dbus_message_get_path(_msg);
    }
    return "";
}

std::string Message::get_interface() const {
    if (is_valid()) {
        return dbus_message_get_interface(_msg);
    }
    return "";
}

std::string Message::get_member() const {
    if (is_valid() && get_type() == Message::Type::METHOD_CALL) {
        return dbus_message_get_member(_msg);
    }
    return "";
}

bool Message::is_signal(const std::string& interface, const std::string& signal_name) const {
    return is_valid() && dbus_message_is_signal(_msg, interface.c_str(), signal_name.c_str());
}

bool Message::is_method_call(const std::string& interface, const std::string& method) const {
    return get_type() == Type::METHOD_CALL && get_interface() == interface && get_member() == method;
}

static const char* type_to_name(int message_type) {
    switch (message_type) {
        case DBUS_MESSAGE_TYPE_SIGNAL:
            return "signal";
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
            return "method call";
        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
            return "method return";
        case DBUS_MESSAGE_TYPE_ERROR:
            return "error";
        default:
            return "(unknown message type)";
    }
}

std::string Message::to_string(bool append_arguments) const {
    if (!is_valid()) {
        return "INVALID";
    }

    std::ostringstream oss;

    const char* sender;
    const char* destination;

    sender = dbus_message_get_sender(_msg);
    sender = sender ? sender : "(null)";
    destination = dbus_message_get_destination(_msg);
    destination = destination ? destination : "(null)";

    oss << "[" << _unique_id << "] " << type_to_name(dbus_message_get_type(_msg));
    oss << "[" << sender << "->" << destination << "] ";
    oss << dbus_message_get_path(_msg) << " " << dbus_message_get_interface(_msg) << " "
        << dbus_message_get_member(_msg);

    if (get_type() == Message::Type::METHOD_CALL && append_arguments) {
        oss << std::endl;
        oss << "Arguments: " << std::endl;
        for (const auto& arg : _arguments) {
            oss << arg.represent();
        }
    }
    return oss.str();
}

Holder Message::extract() {
    if (!is_valid()) {
        return Holder();
    }

    if (!_is_extracted) {
        if (!_iter_initialized) {
            extract_reset();
        }
        _extracted = _extract_generic(&_iter);
        _is_extracted = true;
    }
    return _extracted;
}

void Message::extract_reset() {
    if (is_valid()) {
        dbus_message_iter_init(_msg, &_iter);
        _iter_initialized = true;
    }
}

bool Message::extract_has_next() { return _iter_initialized && dbus_message_iter_has_next(&_iter); }

void Message::extract_next() {
    if (extract_has_next()) {
        dbus_message_iter_next(&_iter);
        _is_extracted = false;
    }
}

Holder Message::_extract_bytearray(DBusMessageIter* iter) {
    const unsigned char* bytes;
    int len;
    dbus_message_iter_get_fixed_array(iter, &bytes, &len);
    Holder holder_array = Holder::create_array();
    for (int i = 0; i < len; i++) {
        holder_array.array_append(Holder::create_byte(bytes[i]));
    }
    return holder_array;
}

Holder Message::_extract_array(DBusMessageIter* iter) {
    Holder holder_array = Holder::create_array();
    _indent += 1;
    int current_type = dbus_message_iter_get_arg_type(iter);
    if (current_type == DBUS_TYPE_BYTE) {
        holder_array = _extract_bytearray(iter);
    } else {
        while ((current_type = dbus_message_iter_get_arg_type(iter)) != DBUS_TYPE_INVALID) {
            Holder h = _extract_generic(iter);
            if (h.type() != Holder::NONE) {
                holder_array.array_append(h);
            }
            dbus_message_iter_next(iter);
        }
    }
    _indent -= 1;
    return holder_array;
}

Holder Message::_extract_dict(DBusMessageIter* iter) {
    bool holder_initialized = false;
    Holder holder_dict;
    _indent += 1;
    int current_type;

    // Loop through all dictionary entries.
    while ((current_type = dbus_message_iter_get_arg_type(iter)) != DBUS_TYPE_INVALID) {
        // Access the dictionary entry
        DBusMessageIter sub;
        dbus_message_iter_recurse(iter, &sub);

        // Extract the data from the dictionary entry
        Holder key = _extract_generic(&sub);
        dbus_message_iter_next(&sub);
        Holder value = _extract_generic(&sub);

        // Add the data to the dictionary
        if (!holder_initialized) {
            holder_dict = Holder::create_dict();
            holder_initialized = true;
        }

        holder_dict.dict_append(key.type(), key.get_contents(), value);
        dbus_message_iter_next(iter);
    }
    _indent -= 1;
    return holder_dict;
}

Holder Message::_extract_generic(DBusMessageIter* iter) {
    int current_type = dbus_message_iter_get_arg_type(iter);
    if (current_type != DBUS_TYPE_INVALID) {
        switch (current_type) {
            case DBUS_TYPE_BYTE: {
                uint8_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_byte(contents);
            }
            case DBUS_TYPE_BOOLEAN: {
                bool contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_boolean(contents);
            }
            case DBUS_TYPE_INT16: {
                int16_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_int16(contents);
            }
            case DBUS_TYPE_UINT16: {
                uint16_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_uint16(contents);
            }
            case DBUS_TYPE_INT32: {
                int32_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_int32(contents);
            }
            case DBUS_TYPE_UINT32: {
                uint32_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_uint32(contents);
            }
            case DBUS_TYPE_INT64: {
                int64_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_int64(contents);
            }
            case DBUS_TYPE_UINT64: {
                uint64_t contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_uint64(contents);
            }
            case DBUS_TYPE_DOUBLE: {
                double contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_double(contents);
            }
            case DBUS_TYPE_STRING: {
                char* contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_string(contents);
            }
            case DBUS_TYPE_OBJECT_PATH: {
                char* contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_object_path(contents);
            }
            case DBUS_TYPE_SIGNATURE: {
                char* contents;
                dbus_message_iter_get_basic(iter, &contents);
                return Holder::create_signature(contents);
            }
            case DBUS_TYPE_ARRAY: {
                DBusMessageIter sub;
                dbus_message_iter_recurse(iter, &sub);
                int sub_type = dbus_message_iter_get_arg_type(&sub);
                if (sub_type == DBUS_TYPE_DICT_ENTRY) {
                    return _extract_dict(&sub);
                } else {
                    return _extract_array(&sub);
                }
            }
            case DBUS_TYPE_VARIANT: {
                DBusMessageIter sub;
                dbus_message_iter_recurse(iter, &sub);
                _indent += 1;
                Holder h = _extract_generic(&sub);
                _indent -= 1;
                return h;
            }
        }
    }
    return Holder();
}

Message Message::from_retained(DBusMessage* msg) {
    Message message;
    if (msg) {
        dbus_message_ref(msg);
        message._msg = msg;
        message._unique_id = _creation_counter++;
    }
    return message;
}

Message Message::from_acquired(DBusMessage* msg) {
    Message message;
    if (msg) {
        message._msg = msg;
        message._unique_id = _creation_counter++;
    }
    return message;
}

Message Message::create_method_call(const std::string& bus_name, const std::string& path, const std::string& interface,
                                    const std::string& method) {
    DBusMessage* msg = dbus_message_new_method_call(bus_name.c_str(), path.c_str(), interface.c_str(), method.c_str());
    return Message::from_acquired(msg);
}

Message Message::create_method_return(const Message& msg) {
    DBusMessage* msg_return = dbus_message_new_method_return(msg._msg);
    return Message::from_acquired(msg_return);
}

Message Message::create_error(const Message& msg, const std::string& error_name, const std::string& error_message) {
    DBusMessage* msg_error = dbus_message_new_error(msg._msg, error_name.c_str(), error_message.c_str());
    return Message::from_acquired(msg_error);
}

Message Message::create_signal(const std::string& path, const std::string& interface, const std::string& signal) {
    DBusMessage* msg_signal = dbus_message_new_signal(path.c_str(), interface.c_str(), signal.c_str());
    return Message::from_acquired(msg_signal);
}

void Message::_invalidate() {
    _unique_id = INVALID_UNIQUE_ID;
    _msg = nullptr;
    _iter_initialized = false;
    _is_extracted = false;
    _extracted = Holder();

#ifdef DBUS_MESSAGE_ITER_INIT_CLOSED
    _iter = DBUS_MESSAGE_ITER_INIT_CLOSED;
#else
    // For older versions of DBus, DBUS_MESSAGE_ITER_INIT_CLOSED is not defined.
    _iter = DBusMessageIter();
#endif
    _arguments.clear();
}

void Message::_safe_delete() {
    if (is_valid()) {
        dbus_message_unref(this->_msg);
        _invalidate();
    }
}