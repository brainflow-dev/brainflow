#pragma once

#include <simpleble/Characteristic.h>
#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class ServiceBase {
  public:
    ServiceBase(const BluetoothUUID& uuid, std::vector<Characteristic>& characteristics);
    virtual ~ServiceBase() = default;

    BluetoothUUID uuid();
    std::vector<Characteristic> characteristics();

  protected:
    BluetoothUUID uuid_;
    std::vector<Characteristic> characteristics_;
};

}  // namespace SimpleBLE
