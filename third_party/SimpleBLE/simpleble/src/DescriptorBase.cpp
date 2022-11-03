#include <simpleble/Descriptor.h>

#include "DescriptorBase.h"
#include "DescriptorBuilder.h"

using namespace SimpleBLE;

DescriptorBase::DescriptorBase(const BluetoothUUID& uuid) : uuid_(uuid) {}

BluetoothUUID DescriptorBase::uuid() { return uuid_; }