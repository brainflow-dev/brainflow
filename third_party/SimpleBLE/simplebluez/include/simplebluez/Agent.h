#pragma once

#include <simplebluez/interfaces/Agent1.h>
#include <simpledbus/advanced/Proxy.h>

namespace SimpleBluez {

class Agent : public SimpleDBus::Proxy {
  public:
    typedef enum {
        DisplayOnly,
        DisplayYesNo,
        KeyboardOnly,
        NoInputNoOutput,
        KeyboardDisplay,
    } Capabilities;

    Agent(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Agent() = default;

    // ----- PROPERTIES -----
    std::string capabilities() const;
    void set_capabilities(Capabilities capabilities);

    // ----- METHODS -----
    void set_on_request_pin_code(std::function<std::string()> callback);
    void clear_on_request_pin_code();

    void set_on_display_pin_code(std::function<bool(const std::string&)> callback);
    void clear_on_display_pin_code();

    void set_on_request_passkey(std::function<uint32_t()> callback);
    void clear_on_request_passkey();

    void set_on_display_passkey(std::function<void(uint32_t, uint16_t)> callback);
    void clear_on_display_passkey();

    void set_on_request_confirmation(std::function<bool(uint32_t)> callback);
    void clear_on_request_confirmation();

    void set_on_request_authorization(std::function<bool()> callback);
    void clear_on_request_authorization();

    void set_on_authorize_service(std::function<bool(const std::string&)> callback);
    void clear_on_authorize_service();

  private:
    std::shared_ptr<Agent1> agent1();
    Capabilities _capabilities;
};

}  // namespace SimpleBluez
