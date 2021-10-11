#pragma once

#include "simpledbus/SimpleDBus.h"

#include "interfaces/GattService1.h"

#include "BluezGattCharacteristic.h"

#include <memory>
#include <string>
#include <vector>

class BluezGattService : public GattService1 {
  private:
    SimpleDBus::Connection* _conn;
    std::string _path;

    std::map<std::string, std::shared_ptr<BluezGattCharacteristic>> gatt_characteristics;
    void add_interface(std::string interface_name, SimpleDBus::Holder options);

  public:
    BluezGattService(SimpleDBus::Connection* conn, std::string path, SimpleDBus::Holder options);
    ~BluezGattService();

    bool add_path(std::string path, SimpleDBus::Holder options);
    bool remove_path(std::string path, SimpleDBus::Holder options);
    bool process_received_signal(SimpleDBus::Message& message);

    std::vector<std::string> get_characteristic_list();
    std::shared_ptr<BluezGattCharacteristic> get_characteristic(std::string char_uuid);
};
