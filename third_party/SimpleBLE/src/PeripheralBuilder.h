#pragma once

#include <simpleble/Peripheral.h>
#include <memory>

namespace SimpleBLE {

/**
 * @brief Helper class to build a Peripheral object.
 *
 * @details This class provides access to the protected properties of Peripheral
 *          and acts as a constructor, avoiding the need to expose any unneeded
 *          functions to the user.
 *
 */
class PeripheralBuilder : public Peripheral {
  public:
    PeripheralBuilder(std::shared_ptr<PeripheralBase> internal);
    virtual ~PeripheralBuilder() = default;
};

}  // namespace SimpleBLE
