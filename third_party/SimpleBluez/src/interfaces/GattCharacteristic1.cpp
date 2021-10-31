#include "GattCharacteristic1.h"

#include "simpledbus/base/Logger.h"

const std::string GattCharacteristic1::_interface_name = "org.bluez.GattCharacteristic1";

GattCharacteristic1::GattCharacteristic1(SimpleDBus::Connection* conn, std::string path)
    : _conn(conn), _path(path), _notifying(false), Properties{conn, "org.bluez", path}, PropertyHandler(path) {}

GattCharacteristic1::~GattCharacteristic1() {}

void GattCharacteristic1::add_option(std::string option_name, SimpleDBus::Holder value) {
    if (option_name == "UUID") {
        _uuid = value.get_string();
    } else if (option_name == "Value") {
        _value.clear();
        auto value_array = value.get_array();
        LOG_F(VERBOSE_2, "%s -> Value Length: %d", _path.c_str(), value_array.size());
        for (auto& elem : value_array) {
            _value.push_back(elem.get_byte());
        }
        if (ValueChanged) {
            LOG_F(VERBOSE_2, "%s -> ValueChanged\n%s", _path.c_str(), value.represent().c_str());
            ValueChanged(_value);
        }
    } else if (option_name == "Notifying") {
        _notifying = value.get_boolean();
    }
}

void GattCharacteristic1::remove_option(std::string option_name) {}

std::string GattCharacteristic1::get_uuid() { return _uuid; }

std::vector<uint8_t> GattCharacteristic1::get_value() { return _value; }

void GattCharacteristic1::StartNotify() {
    if (!_notifying) {
        LOG_F(DEBUG, "%s -> StartNotify", _path.c_str());
        auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StartNotify");
        _conn->send_with_reply_and_block(msg);
    } else {
        LOG_F(WARN, "%s is already notifying...", _path.c_str());
    }
}
void GattCharacteristic1::StopNotify() {
    if (_notifying) {
        LOG_F(DEBUG, "%s -> StopNotify", _path.c_str());
        auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StopNotify");
        _conn->send_with_reply_and_block(msg);
    } else {
        LOG_F(WARN, "%s was not notifying...", _path.c_str());
    }
}

void GattCharacteristic1::WriteValue(SimpleDBus::Holder value, SimpleDBus::Holder options) {
    LOG_F(DEBUG, "%s -> WriteValue", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "WriteValue");
    msg.append_argument(value, "ay");
    msg.append_argument(options, "a{sv}");
    _conn->send_with_reply_and_block(msg);
}

SimpleDBus::Holder GattCharacteristic1::ReadValue(SimpleDBus::Holder options) {
    LOG_F(DEBUG, "%s -> ReadValue", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "ReadValue");
    msg.append_argument(options, "a{sv}");
    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder value = reply_msg.extract();
    return value;
}

void GattCharacteristic1::write_request(const uint8_t* data, uint16_t length) {
    SimpleDBus::Holder value = SimpleDBus::Holder::create_array();
    for (uint16_t i = 0; i < length; i++) {
        value.array_append(SimpleDBus::Holder::create_byte(data[i]));
    }
    SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
    options.dict_append("type", SimpleDBus::Holder::create_string("request"));
    WriteValue(value, options);
}

void GattCharacteristic1::write_command(const uint8_t* data, uint16_t length) {
    SimpleDBus::Holder value = SimpleDBus::Holder::create_array();
    for (uint16_t i = 0; i < length; i++) {
        value.array_append(SimpleDBus::Holder::create_byte(data[i]));
    }
    SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
    options.dict_append("type", SimpleDBus::Holder::create_string("command"));
    WriteValue(value, options);
}

bool GattCharacteristic1::Property_Notifying() {
    auto value = Get(_interface_name, "Notifying");
    add_option("Notifying", value);
    return _notifying;
}

std::vector<uint8_t> GattCharacteristic1::Property_Value() {
    auto value = Get(_interface_name, "Value");
    add_option("Value", value);
    return _value;
}

void GattCharacteristic1::Action_StartNotify() {
    LOG_F(DEBUG, "%s -> StartNotify", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StartNotify");
    _conn->send_with_reply_and_block(msg);
}

void GattCharacteristic1::Action_StopNotify() {
    LOG_F(DEBUG, "%s -> StopNotify", _path.c_str());
    auto msg = SimpleDBus::Message::create_method_call("org.bluez", _path, _interface_name, "StopNotify");
    _conn->send_with_reply_and_block(msg);
}
