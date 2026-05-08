#include "DescriptorBase.h"

using namespace SimpleBLE;

DescriptorBase::DescriptorBase(const BluetoothUUID& uuid) : uuid_(uuid) {}

BluetoothUUID DescriptorBase::uuid() { return uuid_; }
