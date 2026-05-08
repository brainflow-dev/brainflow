#pragma once

#include <memory>
#include <vector>

#include <simpleble/export.h>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>
#include "simpleble/Characteristic.h"

namespace SimpleBLE {

class ServiceBase;

class SIMPLEBLE_EXPORT Service {
  public:
    Service() = default;
    virtual ~Service() = default;

    bool initialized() const;

    BluetoothUUID uuid();
    ByteArray data();
    std::vector<Characteristic> characteristics();

  protected:
    const ServiceBase* operator->() const;
    ServiceBase* operator->();

    std::shared_ptr<ServiceBase> internal_;
};

}  // namespace SimpleBLE
