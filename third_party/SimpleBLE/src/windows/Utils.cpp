#include "Utils.h"

#include <iomanip>
#include <sstream>

#define MAC_ADDRESS_STR_LENGTH (size_t)17  // Two chars per byte, 5 chars for colon

namespace SimpleBLE {

std::string _mac_address_to_str(uint64_t mac_address) {
    uint8_t* mac_address_ptr = (uint8_t*)&mac_address;
    char mac_address_str[MAC_ADDRESS_STR_LENGTH + 1] = {0};  // Include null terminator.

    int position = 0;
    position += sprintf(&mac_address_str[position], "%02x:%02x:%02x:", mac_address_ptr[5], mac_address_ptr[4],
                        mac_address_ptr[3]);
    position += sprintf(&mac_address_str[position], "%02x:%02x:%02x", mac_address_ptr[2], mac_address_ptr[1],
                        mac_address_ptr[0]);
    return std::string((const char*)mac_address_str);
}

uint64_t _str_to_mac_address(std::string mac_address) {
    // TODO: Validate input - Expected Format: XX:XX:XX:XX:XX:XX
    uint64_t mac_address_number = 0;
    uint8_t* mac_address_ptr = (uint8_t*)&mac_address_number;
    sscanf(&mac_address.c_str()[0], "%02hhx:%02hhx:%02hhx:", &mac_address_ptr[5], &mac_address_ptr[4],
           &mac_address_ptr[3]);
    sscanf(&mac_address.c_str()[9], "%02hhx:%02hhx:%02hhx:", &mac_address_ptr[2], &mac_address_ptr[1],
           &mac_address_ptr[0]);
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

    guid.Data1 = std::strtoul(clean_uuid.substr(0, 8).c_str(), nullptr, 16);
    guid.Data2 = std::strtoul(clean_uuid.substr(8, 4).c_str(), nullptr, 16);
    guid.Data3 = std::strtoul(clean_uuid.substr(12, 4).c_str(), nullptr, 16);
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
