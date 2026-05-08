/*
 * Thread-safe map implementation with custom comparator support
 */
#ifndef KVN_SAFE_MAP_HPP
#define KVN_SAFE_MAP_HPP

#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace kvn {

template <typename _Key, typename _Value, typename _Compare = std::less<_Key>>
class safe_map {
  public:
    safe_map() = default;
    explicit safe_map(const _Compare& comp) : _map(comp) {}

    // Remove copy constructor and copy assignment
    safe_map(const safe_map&) = delete;
    safe_map& operator=(const safe_map&) = delete;

    // Move operations
    safe_map(safe_map&&) noexcept = default;
    safe_map& operator=(safe_map&&) noexcept = default;

    // Add this inside the safe_map class
    _Value& operator[](const _Key& key) {
        std::scoped_lock lock(_mutex);
        return _map[key];
    }

    const _Value& operator[](const _Key& key) const {
        std::scoped_lock lock(_mutex);
        return _map[key];
    }

    void insert(const _Key& key, const _Value& value) {
        std::scoped_lock lock(_mutex);
        _map.insert_or_assign(key, value);
    }

    void insert(_Key&& key, _Value&& value) {
        std::scoped_lock lock(_mutex);
        _map.insert_or_assign(std::move(key), std::move(value));
    }

    bool erase(const _Key& key) {
        std::scoped_lock lock(_mutex);
        return _map.erase(key) > 0;
    }

    std::optional<_Value> get(const _Key& key) const {
        std::scoped_lock lock(_mutex);
        auto it = _map.find(key);
        if (it != _map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool contains(const _Key& key) const {
        std::scoped_lock lock(_mutex);
        return _map.contains(key);
    }

    size_t size() const {
        std::scoped_lock lock(_mutex);
        return _map.size();
    }

    void clear() {
        std::scoped_lock lock(_mutex);
        _map.clear();
    }

    template <typename _Func>
    void atomic_update(const _Key& key, _Func update_func) {
        std::scoped_lock lock(_mutex);
        auto it = _map.find(key);
        if (it != _map.end()) {
            update_func(it->second);
        }
    }

  protected:
    mutable std::recursive_mutex _mutex;
    std::map<_Key, _Value, _Compare> _map;
};

}  // namespace kvn

#endif  // KVN_SAFE_MAP_HPP