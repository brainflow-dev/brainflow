#include "ServiceBuilder.h"

#include "ServiceBase.h"

using namespace SimpleBLE;

ServiceBuilder::ServiceBuilder(const BluetoothUUID& uuid, std::vector<Characteristic> characteristics) {
    internal_ = std::make_shared<ServiceBase>(uuid, characteristics);
}