#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <simpleble/Adapter.h>
#include <simpleble/export.h>

namespace SimpleBLE {

class BackendBase;

/**
 * Backend.
 *
 * NOTE: backends must not implement this class directly, but instead derive
 * from BackendBase, defined in Backend.h.
 *
 * This frontend class is hidden from users for the time being. Once the API
 * stabilizes, this class will be exposed to users.
 */
class SIMPLEBLE_EXPORT Backend {
  public:
    Backend() = default;
    virtual ~Backend() = default;

    bool initialized() const;

    /**
     * Get a list of all available adapters from this backend.
     */
    std::vector<Adapter> get_adapters();

    /**
     * Check if Bluetooth is enabled for this backend.
     */
    bool bluetooth_enabled();

    /**
     * Get the first backend that has Bluetooth enabled.
     *
     * @return The first backend that has Bluetooth enabled, or std::nullopt if
     *    no backends have Bluetooth enabled.
     */
    static std::optional<Backend> first_bluetooth_enabled();

    /**
     * Get the name of the backend.
     */
    std::string name() const noexcept;

    /**
     * Get all available backends.
     *
     * This will cause backends to be instantiated/initialized.
     */
    static std::vector<Backend> get_backends();

  protected:
    BackendBase* operator->();
    const BackendBase* operator->() const;

    std::shared_ptr<BackendBase> internal_;
};

/**
 * Get the enabled backend.
 *
 * Note: this function should not be part of the public API and will be deleted
 * once multiple backends are supported.
 */
Backend get_enabled_backend();

}  // namespace SimpleBLE
