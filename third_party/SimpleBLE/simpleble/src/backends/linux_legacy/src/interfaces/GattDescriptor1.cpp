#include "simplebluezlegacy/interfaces/GattDescriptor1.h"

using namespace SimpleBluezLegacy;

const SimpleDBusLegacy::AutoRegisterInterface<GattDescriptor1> GattDescriptor1::registry{
    "org.bluez.GattDescriptor1",
    // clang-format off
    [](std::shared_ptr<SimpleDBusLegacy::Connection> conn, const std::string& bus_name, const std::string& path, const SimpleDBusLegacy::Holder& options) -> std::shared_ptr<SimpleDBusLegacy::Interface> {
        return std::static_pointer_cast<SimpleDBusLegacy::Interface>(std::make_shared<GattDescriptor1>(conn, path));
    }
    // clang-format on
};

GattDescriptor1::GattDescriptor1(std::shared_ptr<SimpleDBusLegacy::Connection> conn, std::string path)
    : SimpleDBusLegacy::Interface(conn, "org.bluez", path, "org.bluez.GattDescriptor1") {}

GattDescriptor1::~GattDescriptor1() { OnValueChanged.unload(); }

void GattDescriptor1::WriteValue(const ByteArray& value) {
    SimpleDBusLegacy::Holder value_data = SimpleDBusLegacy::Holder::create_array();
    for (size_t i = 0; i < value.size(); i++) {
        value_data.array_append(SimpleDBusLegacy::Holder::create_byte(value[i]));
    }

    SimpleDBusLegacy::Holder options = SimpleDBusLegacy::Holder::create_dict();

    auto msg = create_method_call("WriteValue");
    msg.append_argument(value_data, "ay");
    msg.append_argument(options, "a{sv}");
    _conn->send_with_reply_and_block(msg);
}

ByteArray GattDescriptor1::ReadValue() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBusLegacy::Holder options = SimpleDBusLegacy::Holder::create_dict();
    msg.append_argument(options, "a{sv}");

    SimpleDBusLegacy::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBusLegacy::Holder value = reply_msg.extract();
    update_value(value);

    return Value();
}

std::string GattDescriptor1::UUID() {
    // As the UUID property doesn't change, we can cache it
    std::scoped_lock lock(_property_update_mutex);
    return _uuid;
}

ByteArray GattDescriptor1::Value() {
    std::scoped_lock lock(_property_update_mutex);
    return _value;
}

void GattDescriptor1::property_changed(std::string option_name) {
    if (option_name == "UUID") {
        std::scoped_lock lock(_property_update_mutex);
        _uuid = _properties["UUID"].get_string();
    } else if (option_name == "Value") {
        update_value(_properties["Value"]);
        OnValueChanged();
    }
}

void GattDescriptor1::update_value(SimpleDBusLegacy::Holder& new_value) {
    std::scoped_lock lock(_property_update_mutex);
    auto value_array = new_value.get_array();

    char* value_data = new char[value_array.size()];
    for (std::size_t i = 0; i < value_array.size(); i++) {
        value_data[i] = value_array[i].get_byte();
    }
    _value = ByteArray(value_data, value_array.size());
    delete[] value_data;
}
