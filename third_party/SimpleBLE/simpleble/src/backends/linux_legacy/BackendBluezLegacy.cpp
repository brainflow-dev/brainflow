#include "AdapterLinuxLegacy.h"
#include "BackendBase.h"
#include "BackendUtils.h"
#include "CommonUtils.h"

#include <simplebluezlegacy/Bluez.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace SimpleBLE {

class BackendBluezLegacy : public BackendSingleton<BackendBluezLegacy> {
  public:
    BackendBluezLegacy(buildToken);
    virtual ~BackendBluezLegacy();

    SimpleBluezLegacy::Bluez bluez;

    virtual SharedPtrVector<AdapterBase> get_adapters() override;
    virtual bool bluetooth_enabled() override;
    std::string name() const noexcept override;

  private:
    std::thread* async_thread;
    std::atomic_bool async_thread_active;
    void async_thread_function();
};

std::shared_ptr<BackendBase> BACKEND_LINUX_LEGACY() { return BackendBluezLegacy::get(); }

BackendBluezLegacy::BackendBluezLegacy(buildToken) {
    static std::mutex get_mutex;       // Static mutex to ensure thread safety when accessing the logger
    std::scoped_lock lock(get_mutex);  // Unlock the mutex on function return

    bluez.init();
    async_thread_active = true;
    async_thread = new std::thread(&BackendBluezLegacy::async_thread_function, this);
}

BackendBluezLegacy::~BackendBluezLegacy() {
    async_thread_active = false;
    while (!async_thread->joinable()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    async_thread->join();
    delete async_thread;
}

SharedPtrVector<AdapterBase> BackendBluezLegacy::get_adapters() {
    SharedPtrVector<AdapterBase> adapter_list;

    auto internal_adapters = bluez.get_adapters();
    for (auto& adapter : internal_adapters) {
        adapter_list.push_back(std::make_shared<AdapterLinuxLegacy>(adapter));
    }
    return adapter_list;
}

bool BackendBluezLegacy::bluetooth_enabled() {
    bool enabled = false;

    auto internal_adapters = bluez.get_adapters();
    for (auto& adapter : internal_adapters) {
        if (adapter->powered()) {
            enabled = true;
            break;
        }
    }

    return enabled;
}

std::string BackendBluezLegacy::name() const noexcept { return "SimpleBluez Legacy"; }

void BackendBluezLegacy::async_thread_function() {
    SAFE_RUN({ bluez.register_agent(); });

    while (async_thread_active) {
        SAFE_RUN({ bluez.run_async(); });
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

}  // namespace SimpleBLE
