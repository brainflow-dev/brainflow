#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class DescriptorBase {
  public:
    DescriptorBase(const BluetoothUUID& uuid);
    virtual ~DescriptorBase() = default;

    BluetoothUUID uuid();

  protected:
    BluetoothUUID uuid_;
};

}  // namespace SimpleBLE
