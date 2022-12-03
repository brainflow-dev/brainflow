#pragma once

#include <simpleble/Descriptor.h>
#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class CharacteristicBase {
  public:
    CharacteristicBase(const BluetoothUUID& uuid, std::vector<Descriptor>& descriptors);
    virtual ~CharacteristicBase() = default;

    BluetoothUUID uuid();
    std::vector<Descriptor> descriptors();

  protected:
    BluetoothUUID uuid_;
    std::vector<Descriptor> descriptors_;
};

}  // namespace SimpleBLE
