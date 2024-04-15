#include <simpleble/Service.h>

#include "ServiceBase.h"
#include "ServiceBuilder.h"

using namespace SimpleBLE;

ServiceBase::ServiceBase(const BluetoothUUID& uuid) : uuid_(uuid) {}

ServiceBase::ServiceBase(const BluetoothUUID& uuid, const ByteArray& data) : uuid_(uuid), data_(data) {}

ServiceBase::ServiceBase(const BluetoothUUID& uuid, std::vector<Characteristic>& characteristics)
    : uuid_(uuid), characteristics_(characteristics) {}

BluetoothUUID ServiceBase::uuid() { return uuid_; }

ByteArray ServiceBase::data() { return data_; }

std::vector<Characteristic> ServiceBase::characteristics() { return characteristics_; }
