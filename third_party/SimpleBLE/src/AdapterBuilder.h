#pragma once

#include <simpleble/Adapter.h>
#include <memory>

namespace SimpleBLE {

/**
 * @brief Helper class to build a Adapter object.
 *
 * @details This class provides access to the protected properties of Adapter
 *          and acts as a constructor, avoiding the need to expose any unneeded
 *          functions to the user.
 *
 */
class AdapterBuilder : public Adapter {
  public:
    AdapterBuilder(std::shared_ptr<AdapterBase> internal);
    virtual ~AdapterBuilder() = default;
};

}  // namespace SimpleBLE
