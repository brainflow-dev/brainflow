#pragma once

#include <memory>
#include <string>
#include <vector>

namespace SimpleBLE {

class AdapterBase;

class BackendBase {
  public:
    virtual ~BackendBase() = default;

    virtual std::vector<std::shared_ptr<AdapterBase>> get_adapters() = 0;
    virtual bool bluetooth_enabled() = 0;
    virtual std::string name() const noexcept = 0;
};

}  // namespace SimpleBLE
