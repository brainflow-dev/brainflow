#pragma once

#include <memory>

#include <simpleble/export.h>

#include <simpleble/Descriptor.h>
#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class CharacteristicBase;

class SIMPLEBLE_EXPORT Characteristic {
  public:
    Characteristic() = default;
    virtual ~Characteristic() = default;

    bool initialized() const;

    BluetoothUUID uuid();
    std::vector<Descriptor> descriptors();
    std::vector<std::string> capabilities();

    bool can_read();
    bool can_write_request();
    bool can_write_command();
    bool can_notify();
    bool can_indicate();

  protected:
    CharacteristicBase* operator->();
    const CharacteristicBase* operator->() const;

    std::shared_ptr<CharacteristicBase> internal_;
};

}  // namespace SimpleBLE
