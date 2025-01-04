#include <simpledbus/base/Holder.h>
#include <iomanip>
#include <sstream>

#include "dbus/dbus-protocol.h"

using namespace SimpleDBus;

Holder::Holder() {}

Holder::~Holder() {}

bool Holder::operator!=(const Holder& other) const { return !(*this == other); }

bool Holder::operator==(const Holder& other) const {
    if (type() != other.type()) {
        return false;
    }

    switch (type()) {
        case NONE:
            return true;
        case BYTE:
            return get_byte() == other.get_byte();
        case BOOLEAN:
            return get_boolean() == other.get_boolean();
        case INT16:
            return get_int16() == other.get_int16();
        case UINT16:
            return get_uint16() == other.get_uint16();
        case INT32:
            return get_int32() == other.get_int32();
        case UINT32:
            return get_uint32() == other.get_uint32();
        case INT64:
            return get_int64() == other.get_int64();
        case UINT64:
            return get_uint64() == other.get_uint64();
        case DOUBLE:
            return get_double() == other.get_double();
        case STRING:
            return get_string() == other.get_string();
        case OBJ_PATH:
            return get_object_path() == other.get_object_path();
        case SIGNATURE:
            return get_signature() == other.get_signature();
        case ARRAY:
            return get_array() == other.get_array();
        case DICT:
            return (get_dict_uint8() == other.get_dict_uint8()) && (get_dict_uint16() == other.get_dict_uint16()) &&
                   (get_dict_int16() == other.get_dict_int16()) && (get_dict_uint32() == other.get_dict_uint32()) &&
                   (get_dict_int32() == other.get_dict_int32()) && (get_dict_uint64() == other.get_dict_uint64()) &&
                   (get_dict_int64() == other.get_dict_int64()) && (get_dict_string() == other.get_dict_string()) &&
                   (get_dict_object_path() == other.get_dict_object_path()) &&
                   (get_dict_signature() == other.get_dict_signature());
        default:
            return false;
    }
}

Holder::Type Holder::type() const { return _type; }

std::string Holder::_represent_simple() const {
    std::ostringstream output;
    output << std::boolalpha;

    switch (_type) {
        case BOOLEAN:
            output << get_boolean();
            break;
        case BYTE:
            output << (int)get_byte();
            break;
        case INT16:
            output << (int)get_int16();
            break;
        case UINT16:
            output << (int)get_uint16();
            break;
        case INT32:
            output << get_int32();
            break;
        case UINT32:
            output << get_uint32();
            break;
        case INT64:
            output << get_int64();
            break;
        case UINT64:
            output << get_uint64();
            break;
        case DOUBLE:
            output << get_double();
            break;
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            output << get_string();
            break;
    }
    return output.str();
}

std::vector<std::string> Holder::_represent_container() const {
    std::vector<std::string> output_lines;
    switch (_type) {
        case BOOLEAN:
        case BYTE:
        case INT16:
        case UINT16:
        case INT32:
        case UINT32:
        case INT64:
        case UINT64:
        case DOUBLE:
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            output_lines.push_back(_represent_simple());
            break;
        case ARRAY: {
            output_lines.push_back("Array:");
            std::vector<std::string> additional_lines;
            if (holder_array.size() > 0 && holder_array[0]._type == BYTE) {
                // Dealing with an array of bytes, use custom print functionality.
                std::string temp_line = "";
                for (int i = 0; i < holder_array.size(); i++) {
                    // Represent each byte as a hex string
                    std::stringstream stream;
                    stream << std::setfill('0') << std::setw(2) << std::hex << ((int)holder_array[i].get_byte());
                    temp_line += (stream.str() + " ");
                    if ((i + 1) % 32 == 0) {
                        additional_lines.push_back(temp_line);
                        temp_line = "";
                    }
                }
                additional_lines.push_back(temp_line);
            } else {
                for (int i = 0; i < holder_array.size(); i++) {
                    for (auto& line : holder_array[i]._represent_container()) {
                        additional_lines.push_back(line);
                    }
                }
            }
            for (auto& line : additional_lines) {
                output_lines.push_back("  " + line);
            }
            break;
        }
        case DICT:
            output_lines.push_back("Dictionary:");
            for (auto& [key_type_internal, key, value] : holder_dict) {
                output_lines.push_back(_represent_type(key_type_internal, key) + ":");
                auto additional_lines = value._represent_container();
                for (auto& line : additional_lines) {
                    output_lines.push_back("  " + line);
                }
            }
            break;
    }
    return output_lines;
}

std::string Holder::represent() const {
    std::ostringstream output;
    auto output_lines = _represent_container();
    for (auto& output_line : output_lines) {
        output << output_line << std::endl;
    }
    return output.str();
}

std::string Holder::_signature_simple() const {
    switch (_type) {
        case BOOLEAN:
            return DBUS_TYPE_BOOLEAN_AS_STRING;
        case BYTE:
            return DBUS_TYPE_BYTE_AS_STRING;
        case INT16:
            return DBUS_TYPE_INT16_AS_STRING;
        case UINT16:
            return DBUS_TYPE_UINT16_AS_STRING;
        case INT32:
            return DBUS_TYPE_INT32_AS_STRING;
        case UINT32:
            return DBUS_TYPE_UINT32_AS_STRING;
        case INT64:
            return DBUS_TYPE_INT64_AS_STRING;
        case UINT64:
            return DBUS_TYPE_UINT64_AS_STRING;
        case DOUBLE:
            return DBUS_TYPE_DOUBLE_AS_STRING;
        case STRING:
            return DBUS_TYPE_STRING_AS_STRING;
        case OBJ_PATH:
            return DBUS_TYPE_OBJECT_PATH_AS_STRING;
        case SIGNATURE:
            return DBUS_TYPE_SIGNATURE_AS_STRING;
    }
    return "";
}

std::string Holder::_signature_type(Type type) noexcept {
    switch (type) {
        case BOOLEAN:
            return DBUS_TYPE_BOOLEAN_AS_STRING;
        case BYTE:
            return DBUS_TYPE_BYTE_AS_STRING;
        case INT16:
            return DBUS_TYPE_INT16_AS_STRING;
        case UINT16:
            return DBUS_TYPE_UINT16_AS_STRING;
        case INT32:
            return DBUS_TYPE_INT32_AS_STRING;
        case UINT32:
            return DBUS_TYPE_UINT32_AS_STRING;
        case INT64:
            return DBUS_TYPE_INT64_AS_STRING;
        case UINT64:
            return DBUS_TYPE_UINT64_AS_STRING;
        case DOUBLE:
            return DBUS_TYPE_DOUBLE_AS_STRING;
        case STRING:
            return DBUS_TYPE_STRING_AS_STRING;
        case OBJ_PATH:
            return DBUS_TYPE_OBJECT_PATH_AS_STRING;
        case SIGNATURE:
            return DBUS_TYPE_SIGNATURE_AS_STRING;
    }
    return "";
}

std::string Holder::_represent_type(Type type, std::any value) noexcept {
    std::ostringstream output;
    output << std::boolalpha;

    switch (type) {
        case BOOLEAN:
            output << std::any_cast<bool>(value);
            break;
        case BYTE:
            output << std::any_cast<uint8_t>(value);
            break;
        case INT16:
            output << std::any_cast<int16_t>(value);
            break;
        case UINT16:
            output << std::any_cast<uint16_t>(value);
            break;
        case INT32:
            output << std::any_cast<int32_t>(value);
            break;
        case UINT32:
            output << std::any_cast<uint32_t>(value);
            break;
        case INT64:
            output << std::any_cast<int64_t>(value);
            break;
        case UINT64:
            output << std::any_cast<uint64_t>(value);
            break;
        case DOUBLE:
            output << std::any_cast<double>(value);
            break;
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            output << std::any_cast<std::string>(value);
            break;
    }
    return output.str();
}

std::string Holder::signature() const {
    std::string output;
    switch (_type) {
        case BOOLEAN:
        case BYTE:
        case INT16:
        case UINT16:
        case INT32:
        case UINT32:
        case INT64:
        case UINT64:
        case DOUBLE:
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            output = _signature_simple();
            break;
        case ARRAY:
            output = DBUS_TYPE_ARRAY_AS_STRING;
            if (holder_array.size() == 0) {
                output += DBUS_TYPE_VARIANT_AS_STRING;
            } else {
                // Check if all elements of holder_array are the same type
                auto first_type = holder_array[0]._type;
                bool all_same_type = true;
                for (auto& element : holder_array) {
                    if (element._type != first_type) {
                        all_same_type = false;
                        break;
                    }
                }

                if (all_same_type) {
                    output += holder_array[0]._signature_simple();
                } else {
                    output += DBUS_TYPE_VARIANT_AS_STRING;
                }
            }
            break;
        case DICT:
            output = DBUS_TYPE_ARRAY_AS_STRING;
            output += DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING;

            if (holder_dict.size() == 0) {
                output += DBUS_TYPE_STRING_AS_STRING;
                output += DBUS_TYPE_VARIANT_AS_STRING;
            } else {
                // Check if all keys of holder_dict are the same type
                auto first_key_type = std::get<0>(holder_dict[0]);
                bool all_same_key_type = true;
                for (auto& [key_type_internal, key, value] : holder_dict) {
                    if (key_type_internal != first_key_type) {
                        all_same_key_type = false;
                        break;
                    }
                }
                if (all_same_key_type) {
                    output += _signature_type(first_key_type);
                } else {
                    output += DBUS_TYPE_VARIANT_AS_STRING;
                }

                // Check if all values of holder_dict are the same type
                auto first_value_type = std::get<2>(holder_dict[0])._type;
                bool all_same_value_type = true;
                for (auto& [key_type_internal, key, value] : holder_dict) {
                    if (value._type != first_value_type) {
                        all_same_value_type = false;
                        break;
                    }
                }

                if (all_same_value_type) {
                    output += std::get<2>(holder_dict[0])._signature_simple();
                } else {
                    output += DBUS_TYPE_VARIANT_AS_STRING;
                }
            }

            output += DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
            break;
    }
    return output;
}

Holder Holder::create_byte(uint8_t value) {
    Holder h;
    h._type = BYTE;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_boolean(bool value) {
    Holder h;
    h._type = BOOLEAN;
    h.holder_boolean = value;
    return h;
}
Holder Holder::create_int16(int16_t value) {
    Holder h;
    h._type = INT16;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_uint16(uint16_t value) {
    Holder h;
    h._type = UINT16;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_int32(int32_t value) {
    Holder h;
    h._type = INT32;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_uint32(uint32_t value) {
    Holder h;
    h._type = UINT32;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_int64(int64_t value) {
    Holder h;
    h._type = INT64;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_uint64(uint64_t value) {
    Holder h;
    h._type = UINT64;
    h.holder_integer = value;
    return h;
}
Holder Holder::create_double(double value) {
    Holder h;
    h._type = DOUBLE;
    h.holder_double = value;
    return h;
}
Holder Holder::create_string(const std::string& str) {
    Holder h;
    h._type = STRING;
    h.holder_string = str;
    return h;
}
Holder Holder::create_object_path(const ObjectPath& path) {
    Holder h;
    h._type = OBJ_PATH;
    h.holder_string = path;
    return h;
}
Holder Holder::create_signature(const Signature& signature) {
    Holder h;
    h._type = SIGNATURE;
    h.holder_string = signature;
    return h;
}
Holder Holder::create_array() {
    Holder h;
    h._type = ARRAY;
    h.holder_array.clear();
    return h;
}
Holder Holder::create_dict() {
    Holder h;
    h._type = DICT;
    h.holder_dict.clear();
    return h;
}

template <>
Holder Holder::create(bool value) {
    return create_boolean(value);
}

template <>
Holder Holder::create(uint8_t value) {
    return create_byte(value);
}

template <>
Holder Holder::create(int16_t value) {
    return create_int16(value);
}

template <>
Holder Holder::create(uint16_t value) {
    return create_uint16(value);
}

template <>
Holder Holder::create(int32_t value) {
    return create_int32(value);
}

template <>
Holder Holder::create(uint32_t value) {
    return create_uint32(value);
}

template <>
Holder Holder::create(int64_t value) {
    return create_int64(value);
}

template <>
Holder Holder::create(uint64_t value) {
    return create_uint64(value);
}

template <>
Holder Holder::create(double value) {
    return create_double(value);
}

template <>
Holder Holder::create(const std::string& value) {
    return create_string(value);
}

template <>
Holder Holder::create(const ObjectPath& value) {
    return create_object_path(value);
}

template <>
Holder Holder::create(const Signature& value) {
    return create_signature(value);
}

template <>
Holder Holder::create<std::vector<Holder>>() {
    return create_array();
}

template <>
Holder Holder::create<std::map<std::string, Holder>>() {
    return create_dict();
}

std::any Holder::get_contents() const {
    // Only return the contents for simple types
    switch (_type) {
        case BOOLEAN:
            return get_boolean();
        case BYTE:
            return get_byte();
        case INT16:
            return get_int16();
        case UINT16:
            return get_uint16();
        case INT32:
            return get_int32();
        case UINT32:
            return get_uint32();
        case INT64:
            return get_int64();
        case UINT64:
            return get_uint64();
        case DOUBLE:
            return get_double();
        case STRING:
            return get_string();
        case OBJ_PATH:
            return get_object_path();
        case SIGNATURE:
            return get_signature();
        default:
            return std::any();
    }
}

bool Holder::get_boolean() const { return holder_boolean; }

uint8_t Holder::get_byte() const { return (uint8_t)(holder_integer & 0x00000000000000FFL); }

int16_t Holder::get_int16() const { return (int16_t)(holder_integer & 0x000000000000FFFFL); }

uint16_t Holder::get_uint16() const { return (uint16_t)(holder_integer & 0x000000000000FFFFL); }

int32_t Holder::get_int32() const { return (int32_t)(holder_integer & 0x00000000FFFFFFFFL); }

uint32_t Holder::get_uint32() const { return (uint32_t)(holder_integer & 0x00000000FFFFFFFFL); }

int64_t Holder::get_int64() const { return (int64_t)holder_integer; }

uint64_t Holder::get_uint64() const { return holder_integer; }

double Holder::get_double() const { return holder_double; }

std::string Holder::get_string() const { return holder_string; }

std::string Holder::get_object_path() const { return holder_string; }

std::string Holder::get_signature() const { return holder_string; }

std::vector<Holder> Holder::get_array() const { return holder_array; }

std::map<uint8_t, Holder> Holder::get_dict_uint8() const { return _get_dict<uint8_t>(BYTE); }

std::map<uint16_t, Holder> Holder::get_dict_uint16() const { return _get_dict<uint16_t>(UINT16); }

std::map<uint32_t, Holder> Holder::get_dict_uint32() const { return _get_dict<uint32_t>(UINT32); }

std::map<uint64_t, Holder> Holder::get_dict_uint64() const { return _get_dict<uint64_t>(UINT64); }

std::map<int16_t, Holder> Holder::get_dict_int16() const { return _get_dict<int16_t>(INT16); }

std::map<int32_t, Holder> Holder::get_dict_int32() const { return _get_dict<int32_t>(INT32); }

std::map<int64_t, Holder> Holder::get_dict_int64() const { return _get_dict<int64_t>(INT64); }

std::map<std::string, Holder> Holder::get_dict_string() const { return _get_dict<std::string>(STRING); }

std::map<std::string, Holder> Holder::get_dict_object_path() const { return _get_dict<std::string>(OBJ_PATH); }

std::map<std::string, Holder> Holder::get_dict_signature() const { return _get_dict<std::string>(SIGNATURE); }

template <>
bool Holder::get() const {
    return get_boolean();
}

template <>
uint8_t Holder::get() const {
    return get_byte();
}

template <>
int16_t Holder::get() const {
    return get_int16();
}

template <>
uint16_t Holder::get() const {
    return get_uint16();
}

template <>
int32_t Holder::get() const {
    return get_int32();
}

template <>
uint32_t Holder::get() const {
    return get_uint32();
}

template <>
int64_t Holder::get() const {
    return get_int64();
}

template <>
uint64_t Holder::get() const {
    return get_uint64();
}

template <>
double Holder::get() const {
    return get_double();
}

template <>
std::string Holder::get() const {
    return get_string();
}

template <>
std::vector<Holder> Holder::get() const {
    return get_array();
}

template <>
std::map<uint8_t, Holder> Holder::get() const {
    return get_dict_uint8();
}

template <>
std::map<uint16_t, Holder> Holder::get() const {
    return get_dict_uint16();
}

template <>
std::map<uint32_t, Holder> Holder::get() const {
    return get_dict_uint32();
}

template <>
std::map<uint64_t, Holder> Holder::get() const {
    return get_dict_uint64();
}

template <>
std::map<int16_t, Holder> Holder::get() const {
    return get_dict_int16();
}

template <>
std::map<int32_t, Holder> Holder::get() const {
    return get_dict_int32();
}

template <>
std::map<int64_t, Holder> Holder::get() const {
    return get_dict_int64();
}

template <>
std::map<std::string, Holder> Holder::get() const {
    return get_dict_string();
}

template <>
std::map<ObjectPath, Holder> Holder::get() const {
    std::map<ObjectPath, Holder> output;
    for (auto& [key_type_internal, key, value] : holder_dict) {
        if (key_type_internal == OBJ_PATH) {
            output[ObjectPath(std::any_cast<std::string>(key))] = value;
        }
    }
    return output;
}

template <>
std::map<Signature, Holder> Holder::get() const {
    std::map<Signature, Holder> output;
    for (auto& [key_type_internal, key, value] : holder_dict) {
        if (key_type_internal == SIGNATURE) {
            output[Signature(std::any_cast<std::string>(key))] = value;
        }
    }
    return output;
}

void Holder::array_append(Holder holder) { holder_array.push_back(holder); }

void Holder::dict_append(Type key_type, std::any key, Holder value) {
    if (key.type() == typeid(const char*)) {
        key = std::string(std::any_cast<const char*>(key));
    }

    // TODO : VALIDATE THAT THE SPECIFIED KEY TYPE IS CORRECT

    holder_dict.push_back(std::make_tuple(key_type, key, value));
}

template <typename T>
std::map<T, Holder> Holder::_get_dict(Type key_type) const {
    std::map<T, Holder> output;
    for (auto& [key_type_internal, key, value] : holder_dict) {
        if (key_type_internal == key_type) {
            output[std::any_cast<T>(key)] = value;
        }
    }
    return output;
}
