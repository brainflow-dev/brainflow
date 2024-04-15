#pragma once

#include <memory>

#include <simpleble/export.h>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class DescriptorBase;

class SIMPLEBLE_EXPORT Descriptor {
  public:
    Descriptor() = default;
    virtual ~Descriptor() = default;

    BluetoothUUID uuid();

  protected:
    std::shared_ptr<DescriptorBase> internal_;
};

}  // namespace SimpleBLE
