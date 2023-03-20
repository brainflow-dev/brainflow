#pragma once

#include <simpleble/Characteristic.h>
#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class ServiceBase {
  public:
    ServiceBase(const BluetoothUUID& uuid);
    ServiceBase(const BluetoothUUID& uuid, const ByteArray& data);
    ServiceBase(const BluetoothUUID& uuid, std::vector<Characteristic>& characteristics);
    virtual ~ServiceBase() = default;

    BluetoothUUID uuid();
    ByteArray data();
    std::vector<Characteristic> characteristics();

  protected:
    BluetoothUUID uuid_;
    ByteArray data_;
    std::vector<Characteristic> characteristics_;
};

}  // namespace SimpleBLE
