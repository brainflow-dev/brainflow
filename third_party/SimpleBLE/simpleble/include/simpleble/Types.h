#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "kvn/kvn_bytearray.h"

/**
 * @file Types.h
 * @brief Defines types and enumerations used throughout the SimpleBLE library.
 */

namespace SimpleBLE {

using BluetoothAddress = std::string;

// IDEA: Extend BluetoothUUID to include a `uuid` function that
// returns the same string, but provides a homogeneous interface.
using BluetoothUUID = std::string;

/**
 * @typedef ByteArray
 * @brief Represents a byte array using kvn::bytearray from the external library.
 */
using ByteArray = kvn::bytearray;

#ifdef ANDROID
#pragma push_macro("ANDROID")
#undef ANDROID
#define ANDROID_WAS_DEFINED
#endif

enum class OperatingSystem {
    WINDOWS,
    MACOS,
    IOS,
    LINUX,
    ANDROID,
};

#ifdef ANDROID_WAS_DEFINED
#pragma pop_macro("ANDROID")
#undef ANDROID_WAS_DEFINED
#endif

// TODO: Add to_string functions for all enums.
enum BluetoothAddressType : int32_t { PUBLIC = 0, RANDOM = 1, UNSPECIFIED = 2 };

}  // namespace SimpleBLE
