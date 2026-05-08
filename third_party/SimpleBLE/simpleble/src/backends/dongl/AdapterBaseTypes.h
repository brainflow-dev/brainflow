#pragma once

#include <simpleble/Types.h>
#include <cstdint>
#include <map>
#include <string>

namespace SimpleBLE {

struct advertising_data_t {
    std::string identifier;
    BluetoothAddressType address_type;
    BluetoothAddress mac_address;
    bool connectable;
    int16_t rssi;
    int16_t tx_power;

    std::map<uint16_t, ByteArray> manufacturer_data;
    std::map<BluetoothUUID, ByteArray> service_data;
};

}  // namespace SimpleBLE
