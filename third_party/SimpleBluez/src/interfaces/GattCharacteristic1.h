#pragma once

#include "simpledbus/SimpleDBus.h"

#include <string>
#include <vector>

class GattCharacteristic1 : public SimpleDBus::Interfaces::PropertyHandler, public SimpleDBus::Properties {
  private:
    static const std::string _interface_name;

    SimpleDBus::Connection* _conn;
    std::string _path;

    std::string _uuid;
    std::vector<uint8_t> _value;
    bool _notifying;

    void add_option(std::string option_name, SimpleDBus::Holder value);
    void remove_option(std::string option_name);

  public:
    GattCharacteristic1(SimpleDBus::Connection* conn, std::string path);
    ~GattCharacteristic1();

    void StartNotify();
    void StopNotify();

    void WriteValue(SimpleDBus::Holder value, SimpleDBus::Holder options);
    SimpleDBus::Holder ReadValue(SimpleDBus::Holder options);

    std::function<void(std::vector<uint8_t> new_value)> ValueChanged;

    void write_request(const uint8_t* data, uint16_t length);
    void write_command(const uint8_t* data, uint16_t length);

    std::string get_uuid();
    std::vector<uint8_t> get_value();

    void Action_StartNotify();
    void Action_StopNotify();

    bool Property_Notifying();
    std::vector<uint8_t> Property_Value();
};
