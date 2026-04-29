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
    void set_on_request_pin_code(std::function<std::string(const std::string& device_path)> callback);
    void clear_on_request_pin_code();

    void set_on_display_pin_code(std::function<bool(const std::string& device_path, const std::string& pin_code)> callback);
    void clear_on_display_pin_code();

    void set_on_request_passkey(std::function<uint32_t(const std::string& device_path)> callback);
    void clear_on_request_passkey();

    void set_on_display_passkey(std::function<void(const std::string& device_path, uint32_t passkey, uint16_t entered)> callback);
    void clear_on_display_passkey();

    void set_on_request_confirmation(std::function<bool(const std::string& device_path, uint32_t passkey)> callback);
    void clear_on_request_confirmation();

    void set_on_request_authorization(std::function<bool(const std::string& device_path)> callback);
    void clear_on_request_authorization();

    void set_on_authorize_service(std::function<bool(const std::string& device_path, const std::string& uuid)> callback);
    void clear_on_authorize_service();

    void set_on_release(std::function<void()> callback);
    void clear_on_release();

    void set_on_cancel(std::function<void()> callback);
    void clear_on_cancel();
    
    // ----- INTERNAL CALLBACKS -----
    void on_registration() override;

  private:
    std::shared_ptr<Agent1> agent1();
    Capabilities _capabilities;
};

}  // namespace SimpleBluez
