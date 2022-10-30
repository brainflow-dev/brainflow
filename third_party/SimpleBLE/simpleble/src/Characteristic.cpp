#include <simpleble/Characteristic.h>

#include "CharacteristicBase.h"
#include "CharacteristicBuilder.h"

using namespace SimpleBLE;

BluetoothUUID Characteristic::uuid() { return internal_->uuid(); }

std::vector<Descriptor> Characteristic::descriptors() { return internal_->descriptors(); }