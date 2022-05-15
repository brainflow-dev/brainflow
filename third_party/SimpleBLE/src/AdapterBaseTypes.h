#pragma once

#include <simpleble/Types.h>
#include <cstdint>
#include <map>
#include <string>

namespace SimpleBLE {

struct advertising_data_t {
    std::string identifier;
    BluetoothAddress mac_address;
    bool connectable;
    int16_t rssi;

    // TODO: Considering that per advertising report only one manufacturer
    // identifier is supported, maybe it is not necessary to use a map.
    std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data;
};

}  // namespace SimpleBLE
