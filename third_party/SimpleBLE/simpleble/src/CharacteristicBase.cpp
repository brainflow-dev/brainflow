#include <simpleble/Characteristic.h>

#include "CharacteristicBase.h"
#include "CharacteristicBuilder.h"

using namespace SimpleBLE;

CharacteristicBase::CharacteristicBase(const BluetoothUUID& uuid, std::vector<Descriptor>& descriptors)
    : uuid_(uuid), descriptors_(descriptors) {}

BluetoothUUID CharacteristicBase::uuid() { return uuid_; }

std::vector<Descriptor> CharacteristicBase::descriptors() { return descriptors_; }