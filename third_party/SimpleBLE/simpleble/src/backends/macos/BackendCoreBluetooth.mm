#include "../common/BackendBase.h"
#include "../common/BackendUtils.h"
#include "CommonUtils.h"

#include "AdapterMac.h"

namespace SimpleBLE {

class BackendCoreBluetooth : public BackendSingleton<BackendCoreBluetooth> {
  public:
    BackendCoreBluetooth(buildToken);
    virtual ~BackendCoreBluetooth() = default;

    virtual std::vector<std::shared_ptr<AdapterBase>> get_adapters() override;
    virtual bool bluetooth_enabled() override;
    virtual std::string name() const noexcept override;

  private:
    // Apple devices only have a single Bluetooth adapter, so in order to preserve
    // state across multiple instances, a single Adapter object is shared across
    // all users of this backend.
    std::shared_ptr<AdapterMac> _adapter;
};

std::shared_ptr<BackendBase> BACKEND_MACOS() { return BackendCoreBluetooth::get(); }

BackendCoreBluetooth::BackendCoreBluetooth(buildToken) : _adapter{std::make_shared<AdapterMac>()} {}

SharedPtrVector<AdapterBase> BackendCoreBluetooth::get_adapters() {
    SharedPtrVector<AdapterBase> adapter_list;
    adapter_list.push_back(_adapter);
    return adapter_list;
}

bool BackendCoreBluetooth::bluetooth_enabled() {
    // Because CBCentralManager requires an instance of an object to properly operate,
    // we'll fabricate a local AdapterBase object and query it's internal AdapterBaseMacOS
    // to see if Bluetooth is enabled.
    // TODO: Find a better alternative for this.
    return _adapter->bluetooth_enabled();
}

std::string BackendCoreBluetooth::name() const noexcept { return "CoreBluetooth"; }

}  // namespace SimpleBLE
