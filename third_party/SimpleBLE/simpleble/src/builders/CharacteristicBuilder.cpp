#include "CharacteristicBuilder.h"

#include "CharacteristicBase.h"

using namespace SimpleBLE;

CharacteristicBuilder::CharacteristicBuilder(const BluetoothUUID& uuid, std::vector<Descriptor> descriptors) {
    internal_ = std::make_shared<CharacteristicBase>(uuid, descriptors);
}