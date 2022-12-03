#include <simplebluez/Agent.h>

#include <iostream>

using namespace SimpleBluez;

Agent::Agent(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path)
    : Proxy(conn, bus_name, path) {
    _interfaces.emplace(std::make_pair(
        "org.bluez.Agent1", std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Agent1>(_conn, _path))));
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

void Agent::set_on_request_pin_code(std::function<std::string()> callback) {
    agent1()->OnRequestPinCode.load(callback);
}

void Agent::clear_on_request_pin_code() { agent1()->OnRequestPinCode.unload(); }

void Agent::set_on_display_pin_code(std::function<bool(const std::string&)> callback) {
    agent1()->OnDisplayPinCode.load(callback);
}

void Agent::clear_on_display_pin_code() { agent1()->OnDisplayPinCode.unload(); }

void Agent::set_on_request_passkey(std::function<uint32_t()> callback) { agent1()->OnRequestPasskey.load(callback); }

void Agent::clear_on_request_passkey() { agent1()->OnRequestPasskey.unload(); }

void Agent::set_on_display_passkey(std::function<void(uint32_t, uint16_t)> callback) {
    agent1()->OnDisplayPasskey.load(callback);
}

void Agent::clear_on_display_passkey() { agent1()->OnDisplayPasskey.unload(); }

void Agent::set_on_request_confirmation(std::function<bool(uint32_t)> callback) {
    agent1()->OnRequestConfirmation.load(callback);
}

void Agent::clear_on_request_confirmation() { agent1()->OnRequestConfirmation.unload(); }

void Agent::set_on_request_authorization(std::function<bool()> callback) {
    agent1()->OnRequestAuthorization.load(callback);
}

void Agent::clear_on_request_authorization() { agent1()->OnRequestAuthorization.unload(); }

void Agent::set_on_authorize_service(std::function<bool(const std::string&)> callback) {
    agent1()->OnAuthorizeService.load(callback);
}

void Agent::clear_on_authorize_service() { agent1()->OnAuthorizeService.unload(); }

std::shared_ptr<Agent1> Agent::agent1() { return std::dynamic_pointer_cast<Agent1>(interface_get("org.bluez.Agent1")); }
