#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

namespace SimpleDBus {

template <typename T, typename... params>
class Callback {
  public:
    void load(T callback);
    void unload();

    bool is_loaded();
    bool is_running();
    bool is_delete_requested();
    
    void operator()(params... args);

  protected:
    T _callback;
    std::recursive_mutex _mutex;
    
    std::atomic_bool _is_loaded;
    std::atomic_bool _is_running{false};
    std::atomic_bool _delete_requested{false};
};

}  // namespace SimpleDBus
