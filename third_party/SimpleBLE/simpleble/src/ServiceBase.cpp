#include <simpleble/Service.h>

#include "ServiceBase.h"
#include "ServiceBuilder.h"

using namespace SimpleBLE;

ServiceBase::ServiceBase(const BluetoothUUID& uuid, std::vector<Characteristic>& characteristics)
    : uuid_(uuid), characteristics_(characteristics) {}

BluetoothUUID ServiceBase::uuid() { return uuid_; }

std::vector<Characteristic> ServiceBase::characteristics() { return characteristics_; }