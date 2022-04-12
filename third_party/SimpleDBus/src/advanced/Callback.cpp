#include <simpledbus/advanced/Callback.h>

using namespace SimpleDBus;

template <typename T, typename... params>
void Callback<T, params...>::load(T callback) {
    _mutex.lock();
    _callback = callback;
    _mutex.unlock();
    if (callback) {
        _is_loaded = true;
    }
}

template <typename T, typename... params>
void Callback<T, params...>::unload() {
    _is_loaded = false;
    if (_is_running) {
        _delete_requested = true;
    } else {
        _mutex.lock();
        _callback = nullptr;
        _delete_requested = false;
        _mutex.unlock();
    }
}

template <typename T, typename... params>
void Callback<T, params...>::operator()(params... args) {
    _is_running = true;
    if (_is_loaded && !_delete_requested) {
        _mutex.lock();
        if (_callback) {
            _callback(args...);
        }
        _mutex.unlock();
    }
    if (_delete_requested) {
        _mutex.lock();
        _callback = nullptr;
        _mutex.unlock();
        _delete_requested = false;
    }

    _is_running = false;
}

template <typename T, typename... params>
bool Callback<T, params...>::is_loaded() {
    return _is_loaded;
}

template <typename T, typename... params>
bool Callback<T, params...>::is_running() {
    return _is_running;
}

template <typename T, typename... params>
bool Callback<T, params...>::is_delete_requested() {
    return _delete_requested;
}

// Instantiations

namespace SimpleDBus {

template class Callback<std::function<void()>>;
template class Callback<std::function<void(std::string)>, std::string>;

}  // namespace SimpleDBus
