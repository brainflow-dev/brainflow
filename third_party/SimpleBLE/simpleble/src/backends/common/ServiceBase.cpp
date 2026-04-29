
#include "ServiceBase.h"
#include "CommonUtils.h"

using namespace SimpleBLE;

ServiceBase::ServiceBase(const BluetoothUUID& uuid) : uuid_(uuid) {}

ServiceBase::ServiceBase(const BluetoothUUID& uuid, const ByteArray& data) : uuid_(uuid), data_(data) {}

ServiceBase::ServiceBase(const BluetoothUUID& uuid, SharedPtrVector<CharacteristicBase>& characteristics)
    : uuid_(uuid), characteristics_(characteristics) {}

BluetoothUUID ServiceBase::uuid() { return uuid_; }

ByteArray ServiceBase::data() { return data_; }

SharedPtrVector<CharacteristicBase> ServiceBase::characteristics() { return characteristics_; }
