#pragma once

#include <any>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace SimpleDBus {

class Holder;

class Holder {
  public:
    Holder();
    ~Holder();

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
        DICT
    } Type;

    Type type();
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
    static Holder create_dict();

    std::any get_contents();
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
    std::map<uint8_t, Holder> get_dict_uint8();
    std::map<uint16_t, Holder> get_dict_uint16();
    std::map<uint32_t, Holder> get_dict_uint32();
    std::map<uint64_t, Holder> get_dict_uint64();
    std::map<int8_t, Holder> get_dict_int8();
    std::map<int16_t, Holder> get_dict_int16();
    std::map<int32_t, Holder> get_dict_int32();
    std::map<int64_t, Holder> get_dict_int64();
    std::map<std::string, Holder> get_dict_string();
    std::map<std::string, Holder> get_dict_object_path();
    std::map<std::string, Holder> get_dict_signature();

    void dict_append(Type key_type, std::any key, Holder value);
    void array_append(Holder holder);

  private:
    Type _type = NONE;

    bool holder_boolean = false;
    uint64_t holder_integer = 0;
    double holder_double = 0;

    std::string holder_string;
    std::vector<Holder> holder_array;

    // Dictionaries are stored within a vector as a tuple of <key_type, key, holder>
    std::vector<std::tuple<Type, std::any, Holder>> holder_dict;

    std::vector<std::string> _represent_container();
    std::string _represent_simple();
    std::string _signature_simple();

    template <typename T>
    std::map<T, Holder> _get_dict(Type key_type);

    static std::string _signature_type(Type type);
    static std::string _represent_type(Type type, std::any value);
};

}  // namespace SimpleDBus
