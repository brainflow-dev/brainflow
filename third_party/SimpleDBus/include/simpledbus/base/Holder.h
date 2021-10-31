#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace SimpleDBus {

typedef enum {
    NONE,
    BYTE,
    BOOLEAN,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64,
    DOUBLE,
    STRING,
    OBJ_PATH,
    SIGNATURE,
    ARRAY,
    DICT,
    DICT_NUMERIC,
} HolderType;  // TODO: Move into the Holder class.

class Holder;

class Holder {
  private:
    HolderType _type = NONE;

    bool holder_boolean = false;
    uint64_t holder_integer = 0;
    double holder_double = 0;

    std::string holder_string;
    std::vector<Holder> holder_array;
    std::map<std::string, Holder> holder_dict;
    std::map<uint64_t, Holder> holder_dict_numeric;

    std::vector<std::string> _represent_container();
    std::string _represent_simple();
    std::string _signature_simple();

  public:
    Holder();
    ~Holder();

    HolderType type();
    std::string represent();
    std::string signature();

    static Holder create_boolean(bool value);
    static Holder create_byte(uint8_t value);
    static Holder create_int16(int16_t value);
    static Holder create_uint16(uint16_t value);
    static Holder create_int32(int32_t value);
    static Holder create_uint32(uint32_t value);
    static Holder create_int64(int64_t value);
    static Holder create_uint64(uint64_t value);
    static Holder create_double(double value);
    static Holder create_string(const char* str);
    static Holder create_object_path(const char* str);
    static Holder create_signature(const char* str);
    static Holder create_array();
    static Holder create_dict(); // TODO: Is there a better way to handle dictionaries?
    static Holder create_dict_numeric();

    bool get_boolean();
    uint8_t get_byte();
    int16_t get_int16();
    uint16_t get_uint16();
    int32_t get_int32();
    uint32_t get_uint32();
    int64_t get_int64();
    uint64_t get_uint64();
    double get_double();
    std::string get_string();
    std::string get_object_path();
    std::string get_signature();
    std::vector<Holder> get_array();
    std::map<std::string, Holder> get_dict();
    std::map<uint64_t, Holder> get_dict_numeric();

    void array_append(Holder holder);
    void dict_append(std::string key, Holder value);
    void dict_numeric_append(uint64_t key, Holder value);
};

}  // namespace SimpleDBus
