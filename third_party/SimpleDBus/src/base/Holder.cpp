#include <simpledbus/base/Holder.h>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "dbus/dbus-protocol.h"

using namespace SimpleDBus;

Holder::Holder() {}

Holder::~Holder() {}

HolderType Holder::type() { return _type; }

std::string Holder::_represent_simple() {
    std::ostringstream output;

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

std::vector<std::string> Holder::_represent_container() {
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
            for (auto& [key, value] : holder_dict) {
                output_lines.push_back(key);
                auto additional_lines = value._represent_container();
                for (auto& line : additional_lines) {
                    output_lines.push_back("  " + line);
                }
                // output_lines.push_back(value.represent());
            }
            break;
        case DICT_NUMERIC:
            for (auto& [key, value] : holder_dict_numeric) {
                output_lines.push_back(std::to_string(key));
                auto additional_lines = value._represent_container();
                for (auto& line : additional_lines) {
                    output_lines.push_back("  " + line);
                }
                // output_lines.push_back(value.represent());
            }
            break;
    }
    return output_lines;
}

std::string Holder::represent() {
    std::ostringstream output;
    auto output_lines = _represent_container();
    for (auto& output_line : output_lines) {
        output << output_line << std::endl;
    }
    return output.str();
}

std::string Holder::_signature_simple() {
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

std::string Holder::signature() {
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
                // ! FIXME: This is not entirely correct, as not all elements might be equal.
                auto internal_holder = holder_array[0];
                output += internal_holder.signature();
            }
            break;
        case DICT:
        case DICT_NUMERIC:
            output = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING;
            // ! FIXME: This is not entirely correct, we're assuming all key elements are strings.
            output += DBUS_TYPE_VARIANT_AS_STRING;
            output += holder_dict.begin()->second.signature();
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
Holder Holder::create_string(const char* str) {
    Holder h;
    h._type = STRING;
    h.holder_string = std::string(str);
    return h;
}
Holder Holder::create_object_path(const char* str) {
    Holder h;
    h._type = OBJ_PATH;
    h.holder_string = std::string(str);
    return h;
}
Holder Holder::create_signature(const char* str) {
    Holder h;
    h._type = SIGNATURE;
    h.holder_string = std::string(str);
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

Holder Holder::create_dict_numeric() {
    Holder h;
    h._type = DICT_NUMERIC;
    h.holder_dict_numeric.clear();
    return h;
}

bool Holder::get_boolean() { return holder_boolean; }
uint8_t Holder::get_byte() { return (uint8_t)(holder_integer & 0x00000000000000FFL); }
int16_t Holder::get_int16() { return (int16_t)(holder_integer & 0x000000000000FFFFL); }
uint16_t Holder::get_uint16() { return (uint16_t)(holder_integer & 0x000000000000FFFFL); }
int32_t Holder::get_int32() { return (int32_t)(holder_integer & 0x00000000FFFFFFFFL); }
uint32_t Holder::get_uint32() { return (uint32_t)(holder_integer & 0x00000000FFFFFFFFL); }
int64_t Holder::get_int64() { return (int64_t)holder_integer; }
uint64_t Holder::get_uint64() { return holder_integer; }
double Holder::get_double() { return holder_double; }
std::string Holder::get_string() { return holder_string; }
std::string Holder::get_object_path() { return holder_string; }
std::string Holder::get_signature() { return holder_string; }
std::vector<Holder> Holder::get_array() { return holder_array; }
std::map<std::string, Holder> Holder::get_dict() { return holder_dict; }
std::map<uint64_t, Holder> Holder::get_dict_numeric() { return holder_dict_numeric; }

void Holder::array_append(Holder holder) { holder_array.push_back(holder); }
void Holder::dict_append(std::string key, Holder value) { holder_dict[key] = value; }
void Holder::dict_numeric_append(uint64_t key, Holder value) { holder_dict_numeric[key] = value; }
