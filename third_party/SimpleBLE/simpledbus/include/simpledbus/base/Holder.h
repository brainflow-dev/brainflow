#pragma once

#include <any>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace SimpleDBus {

namespace detail {
template <typename T>
inline constexpr bool always_false_v = false;

template <typename T, typename = void>
struct is_map : std::false_type {};

template <typename T>
struct is_map<T, std::void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T>().begin())>> : std::true_type {};

template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

template <typename T, typename = void>
struct is_vector : std::false_type {};

template <typename T>
struct is_vector<T, std::void_t<typename T::value_type, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()),
                                decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>> : std::true_type {};

template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value && !std::is_same_v<T, std::string>;
}  // namespace detail

class ObjectPath {
  public:
    ObjectPath() = default;
    ObjectPath(const std::string& path) : path(path) {}
    ObjectPath(const char* path) : path(path) {}
    operator std::string() const { return path; }
    const char* c_str() const { return path.c_str(); }
    bool operator<(const ObjectPath& other) const { return path < other.path; }
    bool operator==(const ObjectPath& other) const { return path == other.path; }
    bool operator!=(const ObjectPath& other) const { return path != other.path; }

  private:
    std::string path;
};

class Signature {
  public:
    Signature() = default;
    Signature(const std::string& signature) : signature(signature) {}
    Signature(const char* signature) : signature(signature) {}
    operator std::string() const { return signature; }
    const char* c_str() const { return signature.c_str(); }
    bool operator<(const Signature& other) const { return signature < other.signature; }
    bool operator==(const Signature& other) const { return signature == other.signature; }
    bool operator!=(const Signature& other) const { return signature != other.signature; }

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
    void signature_override(const std::string& signature);

    std::any get_contents() const;

    void dict_append(Type key_type, std::any key, Holder value);
    void array_append(Holder holder);

    // Template implementations.
    template <typename T>
    static Holder create() {
        Holder h;
        using U = std::decay_t<T>;
        if constexpr (detail::is_vector_v<U>) {
            h._type = ARRAY;
        } else if constexpr (detail::is_map_v<U>) {
            h._type = DICT;
        }
        return h;
    }

    template <typename T>
    static Holder create(T value) {
        Holder h;
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, bool>) {
            h._type = BOOLEAN;
            h.holder_boolean = value;
        } else if constexpr (std::is_integral_v<U>) {
            h._type = _type_to_enum<U>();
            h.holder_integer = static_cast<uint64_t>(value);
        } else if constexpr (std::is_floating_point_v<U>) {
            h._type = DOUBLE;
            h.holder_double = static_cast<double>(value);
        } else if constexpr (std::is_convertible_v<U, std::string> && !detail::is_vector_v<U>) {
            if constexpr (std::is_same_v<U, ObjectPath>) {
                h._type = OBJ_PATH;
            } else if constexpr (std::is_same_v<U, Signature>) {
                h._type = SIGNATURE;
            } else {
                h._type = STRING;
            }
            h.holder_string = static_cast<std::string>(value);
        } else if constexpr (detail::is_vector_v<U>) {
            h._type = ARRAY;
            for (const auto& item : value) {
                h.array_append(Holder::create(item));
            }
        } else if constexpr (detail::is_map_v<U>) {
            h._type = DICT;
            for (const auto& [key, val] : value) {
                h.dict_append(_type_to_enum<typename U::key_type>(), key, Holder::create(val));
            }
        }
        return h;
    }

    template <typename T>
    T get() const {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, bool>) {
            return holder_boolean;
        } else if constexpr (std::is_integral_v<U>) {
            return static_cast<U>(holder_integer);
        } else if constexpr (std::is_floating_point_v<U>) {
            return static_cast<U>(holder_double);
        } else if constexpr (std::is_same_v<U, std::string>) {
            return holder_string;
        } else if constexpr (std::is_same_v<U, ObjectPath>) {
            return ObjectPath(holder_string);
        } else if constexpr (std::is_same_v<U, Signature>) {
            return Signature(holder_string);
        } else if constexpr (detail::is_vector_v<U>) {
            using V = typename U::value_type;
            if constexpr (std::is_same_v<V, Holder>) {
                return holder_array;
            } else {
                U result;
                for (const auto& h : holder_array) {
                    result.push_back(h.template get<V>());
                }
                return result;
            }
        } else if constexpr (detail::is_map_v<U>) {
            using K = typename U::key_type;
            using V = typename U::mapped_type;
            if constexpr (std::is_same_v<V, Holder>) {
                return _get_dict<K>(_type_to_enum<K>());
            } else {
                std::map<K, V> result;
                for (auto& [key_type_internal, key, value] : holder_dict) {
                    if (key_type_internal == _type_to_enum<K>()) {
                        if constexpr (std::is_same_v<std::decay_t<K>, ObjectPath> || std::is_same_v<std::decay_t<K>, Signature>) {
                            result[K(std::any_cast<std::string>(key))] = value.template get<V>();
                        } else {
                            result[std::any_cast<K>(key)] = value.template get<V>();
                        }
                    }
                }
                return result;
            }
        } else {
            static_assert(detail::always_false_v<U>, "Unsupported type for Holder::get");
        }
    }

  private:
    Type _type = NONE;
    std::optional<std::string> _signature;

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
    std::map<T, Holder> _get_dict(Type key_type) const {
        std::map<T, Holder> output;
        for (auto& [key_type_internal, key, value] : holder_dict) {
            if (key_type_internal == key_type) {
                if constexpr (std::is_same_v<std::decay_t<T>, ObjectPath> || std::is_same_v<std::decay_t<T>, Signature>) {
                    output[T(std::any_cast<std::string>(key))] = value;
                } else {
                    output[std::any_cast<T>(key)] = value;
                }
            }
        }
        return output;
    }

    static std::string _signature_type(Type type) noexcept;
    static std::string _represent_type(Type type, std::any value) noexcept;

    template <typename T>
    static constexpr Type _type_to_enum() {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, uint8_t>) return BYTE;
        if constexpr (std::is_same_v<U, int16_t>) return INT16;
        if constexpr (std::is_same_v<U, uint16_t>) return UINT16;
        if constexpr (std::is_same_v<U, int32_t>) return INT32;
        if constexpr (std::is_same_v<U, uint32_t>) return UINT32;
        if constexpr (std::is_same_v<U, int64_t>) return INT64;
        if constexpr (std::is_same_v<U, uint64_t>) return UINT64;
        if constexpr (std::is_same_v<U, std::string>) return STRING;
        if constexpr (std::is_same_v<U, ObjectPath>) return OBJ_PATH;
        if constexpr (std::is_same_v<U, Signature>) return SIGNATURE;
        return NONE;
    }

    static bool _compare_any(Type type, const std::any& a, const std::any& b);
};

}  // namespace SimpleDBus
