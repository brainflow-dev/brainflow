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

    BluetoothUUID uuid();
    ByteArray data();
    std::vector<Characteristic> characteristics();

  protected:
    std::shared_ptr<ServiceBase> internal_;
};

}  // namespace SimpleBLE
