#pragma once

#include "../common/BackendBase.h"
#include "../common/BackendUtils.h"

namespace SimpleBLE {

class BackendWinRT : public BackendSingleton<BackendWinRT> {
  public:
    BackendWinRT(buildToken);
    virtual ~BackendWinRT() = default;

    virtual std::vector<std::shared_ptr<AdapterBase>> get_adapters() override;
    virtual bool bluetooth_enabled() override;
    virtual std::string name() const noexcept override;
};

}  // namespace SimpleBLE
