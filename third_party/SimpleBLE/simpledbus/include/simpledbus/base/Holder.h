#pragma once

#include <any>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace SimpleDBus {

class ObjectPath {
  public:
    ObjectPath(const std::string& path) : path(path) {}
    ObjectPath(const char* path) : path(path) {}
    operator std::string() const { return path; }
    bool operator<(const ObjectPath& other) const { return path < other.path; }

  private:
    std::string path;
};

class Signature {
  public:
    Signature(const std::string& signature) : signature(signature) {}
    Signature(const char* signature) : signature(signature) {}
    operator std::string() const { return signature; }
    bool operator<(const Signature& other) const { return signature < other.signature; }

  private:
    std::string signature;
};

class Holder;

class Holder {
  public:
    Holder();
    ~Holder();

    bool operator!=(const Holder& rhs) const;
    bool operator==(const Holder& rhs) const;

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

    Type type() const;
    std::string represent() const;
    std::string signature() const;

    // TODO: Deprecate these functions in favor of templated version.
    static Holder create_boolean(bool value);
    static Holder create_byte(uint8_t value);
    static Holder create_int16(int16_t value);
    static Holder create_uint16(uint16_t value);
    static Holder create_int32(int32_t value);
    static Holder create_uint32(uint32_t value);
    static Holder create_int64(int64_t value);
    static Holder create_uint64(uint64_t value);
    static Holder create_double(double value);
    static Holder create_string(const std::string& str);
    static Holder create_object_path(const ObjectPath& path);
    static Holder create_signature(const Signature& signature);
    static Holder create_array();
    static Holder create_dict();

    std::any get_contents() const;

    // TODO: Deprecate these functions in favor of templated version.
    bool get_boolean() const;
    uint8_t get_byte() const;
    int16_t get_int16() const;
    uint16_t get_uint16() const;
    int32_t get_int32() const;
    uint32_t get_uint32() const;
    int64_t get_int64() const;
    uint64_t get_uint64() const;
    double get_double() const;
    std::string get_string() const;
    std::string get_object_path() const;
    std::string get_signature() const;
    std::vector<Holder> get_array() const;
    std::map<uint8_t, Holder> get_dict_uint8() const;
    std::map<uint16_t, Holder> get_dict_uint16() const;
    std::map<uint32_t, Holder> get_dict_uint32() const;
    std::map<uint64_t, Holder> get_dict_uint64() const;
    std::map<int16_t, Holder> get_dict_int16() const;
    std::map<int32_t, Holder> get_dict_int32() const;
    std::map<int64_t, Holder> get_dict_int64() const;
    std::map<std::string, Holder> get_dict_string() const;
    std::map<std::string, Holder> get_dict_object_path() const;
    std::map<std::string, Holder> get_dict_signature() const;

    void dict_append(Type key_type, std::any key, Holder value);
    void array_append(Holder holder);

    // Template speciallizations.
    template <typename T>
    static Holder create();

    template <typename T>
    static Holder create(T value);

    template <typename T>
    T get() const;

  private:
    Type _type = NONE;

    bool holder_boolean = false;
    uint64_t holder_integer = 0;
    double holder_double = 0;

    std::string holder_string;
    std::vector<Holder> holder_array;

    // Dictionaries are stored within a vector as a tuple of <key_type, key, holder>
    std::vector<std::tuple<Type, std::any, Holder>> holder_dict;

    std::vector<std::string> _represent_container() const;
    std::string _represent_simple() const;
    std::string _signature_simple() const;

    template <typename T>
    std::map<T, Holder> _get_dict(Type key_type) const;

    static std::string _signature_type(Type type) noexcept;
    static std::string _represent_type(Type type, std::any value) noexcept;
};

}  // namespace SimpleDBus
