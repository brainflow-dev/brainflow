#include "CharacteristicBuilder.h"

#include "CharacteristicBase.h"

using namespace SimpleBLE;

CharacteristicBuilder::CharacteristicBuilder(const BluetoothUUID& uuid, std::vector<Descriptor> descriptors,
                                             bool can_read, bool can_write_request, bool can_write_command,
                                             bool can_notify, bool can_indicate) {
    internal_ = std::make_shared<CharacteristicBase>(uuid, descriptors, can_read, can_write_request, can_write_command,
                                                     can_notify, can_indicate);
}