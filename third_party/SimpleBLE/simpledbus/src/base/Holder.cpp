#include <simpledbus/base/Holder.h>
#include <iomanip>
#include <sstream>

#include "dbus/dbus-protocol.h"

using namespace SimpleDBus;

Holder::Holder() {}

Holder::~Holder() {}

bool Holder::operator!=(const Holder& other) const { return !(*this == other); }

bool Holder::operator==(const Holder& other) const {
    if (_type != other._type) {
        return false;
    }

    switch (_type) {
        case NONE:
            return true;
        case BYTE:
        case INT16:
        case UINT16:
        case INT32:
        case UINT32:
        case INT64:
        case UINT64:
            return holder_integer == other.holder_integer;
        case BOOLEAN:
            return holder_boolean == other.holder_boolean;
        case DOUBLE:
            return holder_double == other.holder_double;
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            return holder_string == other.holder_string;
        case ARRAY:
            return holder_array == other.holder_array;
        case DICT: {
            if (holder_dict.size() != other.holder_dict.size()) {
                return false;
            }
            for (const auto& [type_a, key_a, val_a] : holder_dict) {
                bool found = false;
                for (const auto& [type_b, key_b, val_b] : other.holder_dict) {
                    if (type_a == type_b && val_a == val_b) {
                        if (_compare_any(type_a, key_a, key_b)) {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) return false;
            }
            return true;
        }
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
            output << get<bool>();
            break;
        case BYTE:
            output << (int)get<uint8_t>();
            break;
        case INT16:
            output << (int)get<int16_t>();
            break;
        case UINT16:
            output << (int)get<uint16_t>();
            break;
        case INT32:
            output << get<int32_t>();
            break;
        case UINT32:
            output << get<uint32_t>();
            break;
        case INT64:
            output << get<int64_t>();
            break;
        case UINT64:
            output << get<uint64_t>();
            break;
        case DOUBLE:
            output << get<double>();
            break;
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            output << get<std::string>();
            break;
        default:
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
                for (size_t i = 0; i < holder_array.size(); i++) {
                    // Represent each byte as a hex string
                    std::stringstream stream;
                    stream << std::setfill('0') << std::setw(2) << std::hex << ((int)holder_array[i].get<uint8_t>());
                    temp_line += (stream.str() + " ");
                    if ((i + 1) % 32 == 0) {
                        additional_lines.push_back(temp_line);
                        temp_line = "";
                    }
                }
                additional_lines.push_back(temp_line);
            } else {
                for (size_t i = 0; i < holder_array.size(); i++) {
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
        default:
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
        default:
            return "";
    }
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
        default:
            return "";
    }
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
        default:
            break;
    }
    return output.str();
}

void Holder::signature_override(const std::string& signature) {
    // TODO: Check that the signature is valid for the Holder type and contents.
    _signature = signature;
}

std::string Holder::signature() const {
    if (_signature) {
        return *_signature;
    }

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

                if (all_same_value_type && first_value_type != ARRAY && first_value_type != DICT) {
                    output += std::get<2>(holder_dict[0])._signature_simple();
                } else {
                    output += DBUS_TYPE_VARIANT_AS_STRING;
                }
            }

            output += DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
            break;
        default:
            break;
    }
    return output;
}

std::any Holder::get_contents() const {
    // Only return the contents for simple types
    switch (_type) {
        case BOOLEAN:
            return get<bool>();
        case BYTE:
            return get<uint8_t>();
        case INT16:
            return get<int16_t>();
        case UINT16:
            return get<uint16_t>();
        case INT32:
            return get<int32_t>();
        case UINT32:
            return get<uint32_t>();
        case INT64:
            return get<int64_t>();
        case UINT64:
            return get<uint64_t>();
        case DOUBLE:
            return get<double>();
        case STRING:
            return get<std::string>();
        case OBJ_PATH:
            return (std::string)get<ObjectPath>();
        case SIGNATURE:
            return (std::string)get<Signature>();
        default:
            return std::any();
    }
}

void Holder::array_append(Holder holder) { holder_array.push_back(holder); }

void Holder::dict_append(Type key_type, std::any key, Holder value) {
    if (key.type() == typeid(const char*)) {
        key = std::string(std::any_cast<const char*>(key));
    }

    // TODO : VALIDATE THAT THE SPECIFIED KEY TYPE IS CORRECT

    holder_dict.push_back(std::make_tuple(key_type, key, value));
}

bool Holder::_compare_any(Type type, const std::any& a, const std::any& b) {
    if (a.type() != b.type()) return false;
    switch (type) {
        case BOOLEAN:
            return std::any_cast<bool>(a) == std::any_cast<bool>(b);
        case BYTE:
            return std::any_cast<uint8_t>(a) == std::any_cast<uint8_t>(b);
        case INT16:
            return std::any_cast<int16_t>(a) == std::any_cast<int16_t>(b);
        case UINT16:
            return std::any_cast<uint16_t>(a) == std::any_cast<uint16_t>(b);
        case INT32:
            return std::any_cast<int32_t>(a) == std::any_cast<int32_t>(b);
        case UINT32:
            return std::any_cast<uint32_t>(a) == std::any_cast<uint32_t>(b);
        case INT64:
            return std::any_cast<int64_t>(a) == std::any_cast<int64_t>(b);
        case UINT64:
            return std::any_cast<uint64_t>(a) == std::any_cast<uint64_t>(b);
        case DOUBLE:
            return std::any_cast<double>(a) == std::any_cast<double>(b);
        case STRING:
        case OBJ_PATH:
        case SIGNATURE:
            return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
        default:
            return false;
    }
}
