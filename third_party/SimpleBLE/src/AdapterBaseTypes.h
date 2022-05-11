#pragma once

#include <simpleble/Types.h>
#include <cstdint>
#include <map>
#include <string>

namespace SimpleBLE {

typedef struct {
    std::string identifier;
    BluetoothAddress mac_address;
    bool connectable;
    int8_t rssi;  // TODO: Might be useful at some point.

    // TODO: Considering that per advertising report only one manufacturer
    // identifier is supported, maybe it is not necessary to use a map.
    std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data;
} advertising_data_t;

}  // namespace SimpleBLE
