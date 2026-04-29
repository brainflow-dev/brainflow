#include <simplebluez/standard/Agent.h>

using namespace SimpleBluez;

Agent::Agent(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {}

void Agent::on_registration() {
    _interfaces.emplace(std::make_pair("org.bluez.Agent1", std::static_pointer_cast<SimpleDBus::Interface>(
                                                               std::make_shared<Agent1>(_conn, shared_from_this()))));
}

std::string Agent::capabilities() const {
    switch (_capabilities) {
        case DisplayOnly:
            return "DisplayOnly";
        case DisplayYesNo:
            return "DisplayYesNo";
        case KeyboardOnly:
            return "KeyboardOnly";
        case NoInputNoOutput:
            return "NoInputNoOutput";
        case KeyboardDisplay:
            return "KeyboardDisplay";
        default:
            return "";
    }
}

void Agent::set_capabilities(Agent::Capabilities capabilities) { _capabilities = capabilities; }

void Agent::set_on_request_pin_code(std::function<std::string(const std::string& device_path)> callback) {
    agent1()->OnRequestPinCode.load(callback);
}

void Agent::clear_on_request_pin_code() { agent1()->OnRequestPinCode.unload(); }

void Agent::set_on_display_pin_code(std::function<bool(const std::string& device_path, const std::string& pin_code)> callback) {
    agent1()->OnDisplayPinCode.load(callback);
}

void Agent::clear_on_display_pin_code() { agent1()->OnDisplayPinCode.unload(); }

void Agent::set_on_request_passkey(std::function<uint32_t(const std::string& device_path)> callback) {
    agent1()->OnRequestPasskey.load(callback);
}

void Agent::clear_on_request_passkey() { agent1()->OnRequestPasskey.unload(); }

void Agent::set_on_display_passkey(std::function<void(const std::string& device_path, uint32_t passkey, uint16_t entered)> callback) {
    agent1()->OnDisplayPasskey.load(callback);
}

void Agent::clear_on_display_passkey() { agent1()->OnDisplayPasskey.unload(); }

void Agent::set_on_request_confirmation(std::function<bool(const std::string& device_path, uint32_t passkey)> callback) {
    agent1()->OnRequestConfirmation.load(callback);
}

void Agent::clear_on_request_confirmation() { agent1()->OnRequestConfirmation.unload(); }

void Agent::set_on_request_authorization(std::function<bool(const std::string& device_path)> callback) {
    agent1()->OnRequestAuthorization.load(callback);
}

void Agent::clear_on_request_authorization() { agent1()->OnRequestAuthorization.unload(); }

void Agent::set_on_authorize_service(std::function<bool(const std::string& device_path, const std::string& uuid)> callback) {
    agent1()->OnAuthorizeService.load(callback);
}

void Agent::clear_on_authorize_service() { agent1()->OnAuthorizeService.unload(); }

void Agent::set_on_release(std::function<void()> callback) { agent1()->OnRelease.load(callback); }

void Agent::clear_on_release() { agent1()->OnRelease.unload(); }

void Agent::set_on_cancel(std::function<void()> callback) { agent1()->OnCancel.load(callback); }

void Agent::clear_on_cancel() { agent1()->OnCancel.unload(); }

std::shared_ptr<Agent1> Agent::agent1() { return std::dynamic_pointer_cast<Agent1>(interface_get("org.bluez.Agent1")); }