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

    bool initialized() const;

    BluetoothUUID uuid();

  protected:
    DescriptorBase* operator->();
    const DescriptorBase* operator->() const;

    std::shared_ptr<DescriptorBase> internal_;
};

}  // namespace SimpleBLE
