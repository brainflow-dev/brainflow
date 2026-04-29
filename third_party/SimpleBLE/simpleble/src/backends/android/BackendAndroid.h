#pragma once

#include "../common/BackendBase.h"
#include "../common/BackendUtils.h"

namespace SimpleBLE {

class AdapterAndroid;

class BackendAndroid : public BackendSingleton<BackendAndroid> {
  public:
    BackendAndroid(buildToken);
    virtual ~BackendAndroid() = default;

    virtual std::vector<std::shared_ptr<AdapterBase>> get_adapters() override;
    virtual bool bluetooth_enabled() override;
    virtual std::string name() const noexcept override;

  private:
    // Android devices only have a single Bluetooth adapter, so in order to preserve
    // state across multiple instances, a single Adapter object is shared across
    // all users of this backend.
    std::shared_ptr<AdapterAndroid> _adapter;
};

}  // namespace SimpleBLE
