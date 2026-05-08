#include "simplebluezlegacy/interfaces/Agent1.h"

using namespace SimpleBluezLegacy;

const SimpleDBusLegacy::AutoRegisterInterface<Agent1> Agent1::registry{
    "org.bluez.Agent1",
    // clang-format off
    [](std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path, const SimpleDBusLegacy::Holder& options) -> std::shared_ptr<SimpleDBusLegacy::Interface> {
        return std::static_pointer_cast<SimpleDBusLegacy::Interface>(std::make_shared<Agent1>(conn, path));
    }
    // clang-format on
};

Agent1::Agent1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path)
    : SimpleDBusLegacy::Interface(conn, "org.bluez", path, "org.bluez.Agent1") {}

void Agent1::message_handle(SimpleDBusLegacy::Message& msg) {
    if (msg.get_type() == SimpleDBusLegacy::Message::Type::METHOD_CALL) {
        // To minimize the amount of repeated code, create a method return object that will be
        // used to send the reply.
        SimpleDBusLegacy::Message reply = SimpleDBusLegacy::Message::create_method_return(msg);

        if (msg.get_member() == "Release") {
            // Nothing to do

        } else if (msg.get_member() == "RequestPinCode") {
            // std::cout << "Agent1::message_handle() RequestPinCode" << std::endl;

            std::string pin_code = "abc123";
            if (OnRequestPinCode) {
                pin_code = OnRequestPinCode();
            }

            if (!pin_code.empty()) {
                reply.append_argument(SimpleDBusLegacy::Holder::create_string(pin_code), DBUS_TYPE_STRING_AS_STRING);
            } else {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "RequestPasskey") {
            // std::cout << "Agent1::message_handle() RequestPasskey" << std::endl;

            int32_t passkey = 123456;
            if (OnRequestPasskey) {
                passkey = OnRequestPasskey();
            }

            if (passkey >= 0 && passkey <= 999999) {
                reply.append_argument(SimpleDBusLegacy::Holder::create_uint32(static_cast<uint32_t>(passkey)),
                                      DBUS_TYPE_UINT32_AS_STRING);
            } else {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "DisplayPinCode") {
            // std::cout << "Agent1::message_handle() DisplayPinCode" << std::endl;
            SimpleDBusLegacy::Holder arg_device = msg.extract();
            SimpleDBusLegacy::Holder arg_pin_code = msg.extract();

            bool success = true;
            if (OnDisplayPinCode) {
                success = OnDisplayPinCode(arg_pin_code.get_string());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "DisplayPasskey") {
            // std::cout << "Agent1::message_handle() DisplayPasskey" << std::endl;
            SimpleDBusLegacy::Holder arg_device = msg.extract();
            SimpleDBusLegacy::Holder arg_passkey = msg.extract();
            SimpleDBusLegacy::Holder arg_entered = msg.extract();

            if (OnDisplayPasskey) {
                OnDisplayPasskey(arg_passkey.get_uint32(), arg_entered.get_uint16());
            }

        } else if (msg.get_member() == "RequestConfirmation") {
            // std::cout << "Agent1::message_handle() RequestConfirmation" << std::endl;
            SimpleDBusLegacy::Holder arg_device = msg.extract();
            SimpleDBusLegacy::Holder arg_passkey = msg.extract();

            bool success = true;
            if (OnRequestConfirmation) {
                success = OnRequestConfirmation(arg_passkey.get_uint32());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "RequestAuthorization") {
            // std::cout << "Agent1::message_handle() RequestAuthorization" << std::endl;
            SimpleDBusLegacy::Holder arg_device = msg.extract();

            bool success = true;
            if (OnRequestAuthorization) {
                success = OnRequestAuthorization();
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "AuthorizeService") {
            // std::cout << "Agent1::message_handle() AuthorizeService" << std::endl;

            SimpleDBusLegacy::Holder arg_device = msg.extract();
            SimpleDBusLegacy::Holder arg_uuid = msg.extract();

            bool success = true;
            if (OnAuthorizeService) {
                success = OnAuthorizeService(arg_uuid.get_string());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "Cancel") {
            // std::cout << "Agent1::message_handle() Cancel" << std::endl;
            // NOTE: Due to the blocking nature of this interface, the Cancel method won't
            //       have any real impact on any of the callbacks, and thus will be ignored.

        } else {
            // std::cout << "Agent1::message_handle() Unknown method: " << msg.get_member() << std::endl;
        }

        _conn->send(reply);
    }
}

void Agent1::reply_error(SimpleDBusLegacy::Message& msg, const std::string& error_name, const std::string& error_message) {
    SimpleDBusLegacy::Message reply = SimpleDBusLegacy::Message::create_error(msg, error_name, error_message);
    _conn->send(reply);
}
