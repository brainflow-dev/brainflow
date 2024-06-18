#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/external/kvn_safe_callback.hpp>

#include <cstdint>
#include <string>

namespace SimpleBluez {

class Agent1 : public SimpleDBus::Interface {
  public:
    Agent1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~Agent1() = default;

    // ----- METHODS -----

    // ----- PROPERTIES -----

    // ----- CALLBACKS -----

    /**
     * @brief This method gets called when the service daemon
     *        needs to get the passkey for an authentication.
     *        The return value should be a string of 1-16 characters
     *        length. The string can be alphanumeric.
     *
     * @note: Invalid values will cause a rejection of the request
     *        be returned.
     */
    kvn::safe_callback<std::string()> OnRequestPinCode;

    /**
     * @brief This method gets called when the service daemon
     *        needs to display a pincode for an authentication.
     *        Note that the PIN will always be a 6-digit number,
     *        zero-padded to 6 digits.
     *
     * @return false if the request should be rejected.
     */
    kvn::safe_callback<bool(const std::string&)> OnDisplayPinCode;

    /**
     * @brief This method gets called when the service daemon
     *        needs to get the passkey for an authentication.
     *        The return value should be a numeric value
     *        between 0 and 999999.
     *
     * @note: Invalid values will cause a rejection of the request
     *        be returned.
     */
    kvn::safe_callback<int32_t()> OnRequestPasskey;

    /**
     * @brief This method gets called when the service daemon
     *        needs to display a passkey for an authentication.
     *        The entered parameter indicates the number of already
     *        typed keys on the remote side.
     */
    kvn::safe_callback<void(uint32_t, uint16_t)> OnDisplayPasskey;

    /**
     * @brief This method gets called when the service daemon
     *        needs to confirm a passkey for an authentication.
     *
     * @return false if the request should be rejected.
     */
    kvn::safe_callback<bool(uint32_t)> OnRequestConfirmation;

    /**
     * @brief This method gets called to request the user to
     *        authorize an incoming pairing attempt which
     *        would in other circumstances trigger the just-works
     *        model, or when the user plugged in a device that
     *        implements cable pairing.
     *
     * @return false if the request should be rejected.
     */
    kvn::safe_callback<bool()> OnRequestAuthorization;

    /**
     * @brief This method gets called when the service daemon
     *        needs to authorize a connection/service request.
     *
     * @return false if the request should be rejected.
     */
    kvn::safe_callback<bool(const std::string&)> OnAuthorizeService;

  protected:
    void message_handle(SimpleDBus::Message& msg) override;

    void reply_error(SimpleDBus::Message& msg, const std::string& error_name, const std::string& error_message);
};

}  // namespace SimpleBluez
