#pragma once

#include <cstdint>
#include <string>

#include <simpleble/Types.h>

#include "winrt/Windows.Storage.Streams.h"
#include "winrt/base.h"

using namespace winrt::Windows::Storage::Streams;

namespace SimpleBLE {

std::string _mac_address_to_str(uint64_t mac_address);
uint64_t _str_to_mac_address(std::string mac_address);

winrt::guid uuid_to_guid(const std::string& uuid);
std::string guid_to_uuid(const winrt::guid& guid);

ByteArray ibuffer_to_bytearray(const IBuffer& buffer);
IBuffer bytearray_to_ibuffer(const ByteArray& array);

}  // namespace SimpleBLE