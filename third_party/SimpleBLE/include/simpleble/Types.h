#pragma once

#include <string>
#include <vector>

namespace SimpleBLE {

using BluetoothAddress = std::string;

// IDEA: Extend BluetoothUUID to include a `uuid` function that
// returns the same string, but provides a homogeneous interface.
using BluetoothUUID = std::string;

// IDEA: Extend ByteArray to be constructed by a vector of bytes
// and pointers to uint8_t.
using ByteArray = std::string;


struct BluetoothService {
    BluetoothUUID uuid;
    std::vector<BluetoothUUID> characteristics;
};

enum class OperatingSystem {
    WINDOWS,
    MACOS,
    LINUX,
};

}  // namespace SimpleBLE
