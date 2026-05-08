#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>
#include <memory>

namespace SimpleBLE {

class CharacteristicBase;

class ServiceBase {
  public:
    ServiceBase(const BluetoothUUID& uuid);
    ServiceBase(const BluetoothUUID& uuid, const ByteArray& data);
    ServiceBase(const BluetoothUUID& uuid, std::vector<std::shared_ptr<CharacteristicBase>>& characteristics);
    virtual ~ServiceBase() = default;

    BluetoothUUID uuid();
    ByteArray data();
    std::vector<std::shared_ptr<CharacteristicBase>> characteristics();

  protected:
    BluetoothUUID uuid_;
    ByteArray data_;
    std::vector<std::shared_ptr<CharacteristicBase>> characteristics_;
};

}  // namespace SimpleBLE
