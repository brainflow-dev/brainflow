#include "Utils.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <sdkddkver.h>
#include "../winrt/roapi.h"

#include "LoggingInternal.h"

#if WDK_NTDDI_VERSION < NTDDI_WIN10_VB
#error "Windows SDK version before 10.0.19041.0 is not supported"
#elif WDK_NTDDI_VERSION == NTDDI_WIN10_VB
// For Windows SDK version before 10.0.19041.0, remap functions to post-10.0.19041.0 versions
#define WINRT_IMPL_CoGetApartmentType WINRT_CoGetApartmentType
#endif

#define MAC_ADDRESS_STR_LENGTH (size_t)17  // Two chars per byte, 5 chars for colon

namespace SimpleBLE {

void initialize_winrt() {
    static bool initialized = false;

    if (initialized) return;
    initialized = true;

    int32_t cotype, qualifier, get_apartment_result;

    get_apartment_result = WINRT_IMPL_CoGetApartmentType(&cotype, &qualifier);
    SIMPLEBLE_LOG_INFO(fmt::format("CoGetApartmentType: cotype={}, qualifier={}, result={:X}", cotype, qualifier,
                                   (uint32_t)get_apartment_result));

    if (cotype == APTTYPE_STA || cotype == APTTYPE_MAINSTA) {
        SIMPLEBLE_LOG_WARN("Single-threaded apartment detected, uninitializing.");
        RoUninitialize();
    }

    winrt::hresult result = RoInitialize(RO_INIT_MULTITHREADED);
    SIMPLEBLE_LOG_INFO(fmt::format("RoInitialize: result={:X}", (uint32_t)result));
}

std::string _mac_address_to_str(uint64_t mac_address) {
    uint8_t* mac_ptr = (uint8_t*)&mac_address;
    char mac_str[MAC_ADDRESS_STR_LENGTH + 1] = {0};  // Include null terminator.

    snprintf(mac_str, MAC_ADDRESS_STR_LENGTH + 1, "%02x:%02x:%02x:%02x:%02x:%02x", mac_ptr[5], mac_ptr[4], mac_ptr[3],
             mac_ptr[2], mac_ptr[1], mac_ptr[0]);
    return std::string(mac_str);
}

uint64_t _str_to_mac_address(std::string mac_str) {
    // TODO: Validate input - Expected Format: XX:XX:XX:XX:XX:XX
    uint64_t mac_address_number = 0;
    uint8_t* mac_ptr = (uint8_t*)&mac_address_number;
    sscanf(mac_str.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac_ptr[5], &mac_ptr[4], &mac_ptr[3],
           &mac_ptr[2], &mac_ptr[1], &mac_ptr[0]);
    return mac_address_number;
}

winrt::guid uuid_to_guid(const std::string& uuid) {
    // TODO: Add proper cleanup / validation
    std::stringstream helper;
    for (int i = 0; i < uuid.length(); i++) {
        if (uuid[i] != '-') {
            helper << uuid[i];
        }
    }
    std::string clean_uuid = helper.str();
    winrt::guid guid;
    uint64_t* data4_ptr = (uint64_t*)guid.Data4;

    guid.Data1 = static_cast<uint32_t>(std::strtoul(clean_uuid.substr(0, 8).c_str(), nullptr, 16));
    guid.Data2 = static_cast<uint16_t>(std::strtoul(clean_uuid.substr(8, 4).c_str(), nullptr, 16));
    guid.Data3 = static_cast<uint16_t>(std::strtoul(clean_uuid.substr(12, 4).c_str(), nullptr, 16));
    *data4_ptr = _byteswap_uint64(std::strtoull(clean_uuid.substr(16, 16).c_str(), nullptr, 16));

    return guid;
}

std::string guid_to_uuid(const winrt::guid& guid) {
    std::stringstream helper;
    // TODO: It might be cleaner to use snprintf instead of string streams.

    for (uint32_t i = 0; i < 4; i++) {
        // * NOTE: We're performing a byte swap!
        helper << std::hex << std::setw(2) << std::setfill('0') << (int)((uint8_t*)&guid.Data1)[3 - i];
    }
    helper << '-';
    for (uint32_t i = 0; i < 2; i++) {
        // * NOTE: We're performing a byte swap!
        helper << std::hex << std::setw(2) << std::setfill('0') << (int)((uint8_t*)&guid.Data2)[1 - i];
    }
    helper << '-';
    for (uint32_t i = 0; i < 2; i++) {
        // * NOTE: We're performing a byte swap!
        helper << std::hex << std::setw(2) << std::setfill('0') << (int)((uint8_t*)&guid.Data3)[1 - i];
    }
    helper << '-';
    for (uint32_t i = 0; i < 2; i++) {
        helper << std::hex << std::setw(2) << std::setfill('0') << (int)guid.Data4[i];
    }
    helper << '-';
    for (uint32_t i = 0; i < 6; i++) {
        helper << std::hex << std::setw(2) << std::setfill('0') << (int)guid.Data4[2 + i];
    }
    return helper.str();
}

ByteArray ibuffer_to_bytearray(const IBuffer& buffer) { return ByteArray((const char*)buffer.data(), buffer.Length()); }

IBuffer bytearray_to_ibuffer(const ByteArray& array) {
    DataWriter writer;
    for (auto& byte : array) {
        writer.WriteByte(byte);
    }
    return writer.DetachBuffer();
}

}  // namespace SimpleBLE
