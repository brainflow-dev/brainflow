/*
 * SPDX-FileCopyrightText: 2022 Kevin Dewald <kevin@dewald.me>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef KVN_SAFE_CALLBACK_HPP
#define KVN_SAFE_CALLBACK_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

namespace kvn {

template <typename _Signature>
class safe_callback;

template <class _Res, class... _ArgTypes>
class safe_callback<_Res(_ArgTypes...)> {
  public:
    virtual ~safe_callback() {
        if (!_is_loaded) {
            return;
        }
        unload();
    };

    void load(std::function<_Res(_ArgTypes...)> callback) {
        if (callback == nullptr) {
            return;
        }
        std::scoped_lock lock(_mutex);
        _callback = std::move(callback);
        _is_loaded = true;
    }

    void unload() {
        std::scoped_lock lock(_mutex);
        _callback = nullptr;
        _is_loaded = false;
    }

    bool is_loaded() const { return _is_loaded; }

    explicit operator bool() const { return is_loaded(); }

    _Res operator()(_ArgTypes... arguments) {
        if (_is_loaded) {
            std::scoped_lock lock(_mutex);
            return _callback(static_cast<_ArgTypes&&>(arguments)...);
        } else {
            return _Res();
        }
    }

  protected:
    std::atomic_bool _is_loaded{false};
    std::function<_Res(_ArgTypes...)> _callback;
    std::recursive_mutex _mutex;
};

}  // namespace kvn

#endif  // KVN_SAFE_CALLBACK_HPP
