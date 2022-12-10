#pragma once

#include <simpleble/Descriptor.h>
#include <memory>

namespace SimpleBLE {

/**
 * @brief Helper class to build a Descriptor object.
 *
 * @details This class provides access to the protected properties of Descriptor
 *          and acts as a constructor, avoiding the need to expose any unneeded
 *          functions to the user.
 *
 */
class DescriptorBuilder : public Descriptor {
  public:
    DescriptorBuilder(const BluetoothUUID& uuid);
    virtual ~DescriptorBuilder() = default;
};

}  // namespace SimpleBLE
