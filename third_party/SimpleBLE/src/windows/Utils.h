#pragma once

#include <cstdint>
#include <string>

#include <simpleble/Types.h>

#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/base.h"

using namespace winrt::Windows;
using namespace winrt::Windows::Storage::Streams;

namespace SimpleBLE {

void initialize_winrt();

std::string _mac_address_to_str(uint64_t mac_address);
uint64_t _str_to_mac_address(std::string mac_address);

winrt::guid uuid_to_guid(const std::string& uuid);
std::string guid_to_uuid(const winrt::guid& guid);

ByteArray ibuffer_to_bytearray(const IBuffer& buffer);
IBuffer bytearray_to_ibuffer(const ByteArray& array);

template <typename async_t>
static auto async_get(async_t const& async) {
    if (async.Status() == Foundation::AsyncStatus::Started) {
        wait_for_completed(async, 0xFFFFFFFF);  // INFINITE
    }
    return async.GetResults();
}

}  // namespace SimpleBLE