#include "simplebluez/interfaces/Agent1.h"

using namespace SimpleBluez;

const SimpleDBus::AutoRegisterInterface<Agent1> Agent1::registry{
    "org.bluez.Agent1",
    // clang-format off
    [](std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy) -> std::shared_ptr<SimpleDBus::Interface> {
        return std::static_pointer_cast<SimpleDBus::Interface>(std::make_shared<Agent1>(conn, proxy));
    }
    // clang-format on
};

Agent1::Agent1(std::shared_ptr<SimpleDBus::Connection> conn, std::shared_ptr<SimpleDBus::Proxy> proxy)
    : SimpleDBus::Interface(conn, proxy, "org.bluez.Agent1") {}

// IMPORTANT: The destructor is defined here (instead of inline) to anchor the vtable to this object file.
// This prevents the linker from stripping this translation unit and ensures the static 'registry' variable is
// initialized at startup.
Agent1::~Agent1() = default;

void Agent1::message_handle(SimpleDBus::Message& msg) {
    if (msg.get_type() == SimpleDBus::Message::Type::METHOD_CALL) {
        // To minimize the amount of repeated code, create a method return object that will be
        // used to send the reply.
        SimpleDBus::Message reply = SimpleDBus::Message::create_method_return(msg);

        if (msg.get_member() == "Release") {
            // Nothing to do
            if (OnRelease) {
                OnRelease();
            }

        } else if (msg.get_member() == "RequestPinCode") {
            // std::cout << "Agent1::message_handle() RequestPinCode" << std::endl;
            SimpleDBus::Holder arg_device = msg.extract();

            std::string pin_code = "abc123";
            if (OnRequestPinCode) {
                pin_code = OnRequestPinCode(arg_device.get<std::string>());
            }

            if (!pin_code.empty()) {
                reply.append_argument(SimpleDBus::Holder::create<std::string>(pin_code), DBUS_TYPE_STRING_AS_STRING);
            } else {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "RequestPasskey") {
            // std::cout << "Agent1::message_handle() RequestPasskey" << std::endl;
            SimpleDBus::Holder arg_device = msg.extract();
            int32_t passkey = 123456;
            if (OnRequestPasskey) {
                passkey = OnRequestPasskey(arg_device.get<std::string>());
            }

            if (passkey >= 0 && passkey <= 999999) {
                reply.append_argument(SimpleDBus::Holder::create<uint32_t>(static_cast<uint32_t>(passkey)),
                                      DBUS_TYPE_UINT32_AS_STRING);
            } else {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "DisplayPinCode") {
            // std::cout << "Agent1::message_handle() DisplayPinCode" << std::endl;
            SimpleDBus::Holder arg_device = msg.extract();
            msg.extract_next();
            SimpleDBus::Holder arg_pin_code = msg.extract();

            bool success = true;
            if (OnDisplayPinCode) {
                success = OnDisplayPinCode(arg_device.get<std::string>(), arg_pin_code.get<std::string>());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "DisplayPasskey") {
            // std::cout << "Agent1::message_handle() DisplayPasskey" << std::endl;
            SimpleDBus::Holder arg_device = msg.extract();
            msg.extract_next();
            SimpleDBus::Holder arg_passkey = msg.extract();
            msg.extract_next();
            SimpleDBus::Holder arg_entered = msg.extract();

            if (OnDisplayPasskey) {
                OnDisplayPasskey(arg_device.get<std::string>(), arg_passkey.get<uint32_t>(),
                                 arg_entered.get<uint16_t>());
            }

        } else if (msg.get_member() == "RequestConfirmation") {
            // std::cout << "Agent1::message_handle() RequestConfirmation" << std::endl;
            SimpleDBus::Holder arg_device = msg.extract();
            msg.extract_next();
            SimpleDBus::Holder arg_passkey = msg.extract();

            bool success = true;
            if (OnRequestConfirmation) {
                success = OnRequestConfirmation(arg_device.get<std::string>(), arg_passkey.get<uint32_t>());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "RequestAuthorization") {
            // std::cout << "Agent1::message_handle() RequestAuthorization" << std::endl;
            SimpleDBus::Holder arg_device = msg.extract();

            bool success = true;
            if (OnRequestAuthorization) {
                success = OnRequestAuthorization(arg_device.get<std::string>());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "AuthorizeService") {
            // std::cout << "Agent1::message_handle() AuthorizeService" << std::endl;

            SimpleDBus::Holder arg_device = msg.extract();
            msg.extract_next();
            SimpleDBus::Holder arg_uuid = msg.extract();

            bool success = true;
            if (OnAuthorizeService) {
                success = OnAuthorizeService(arg_device.get<std::string>(), arg_uuid.get<std::string>());
            }

            if (!success) {
                reply_error(msg, "org.bluez.Error.Rejected", "User rejected the request");
                return;
            }

        } else if (msg.get_member() == "Cancel") {
            // std::cout << "Agent1::message_handle() Cancel" << std::endl;
            // NOTE: Due to the blocking nature of this interface, the Cancel method won't
            //       have any real impact on any of the callbacks, and thus will be ignored.
            if (OnCancel) {
                OnCancel();
            }

        } else {
            // std::cout << "Agent1::message_handle() Unknown method: " << msg.get_member() << std::endl;
        }

        _conn->send(reply);
    }
}

void Agent1::reply_error(SimpleDBus::Message& msg, const std::string& error_name, const std::string& error_message) {
    SimpleDBus::Message reply = SimpleDBus::Message::create_error(msg, error_name, error_message);
    _conn->send(reply);
}
