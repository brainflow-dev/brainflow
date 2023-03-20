#include <simpleble/Service.h>

#include "ServiceBase.h"
#include "ServiceBuilder.h"

using namespace SimpleBLE;

BluetoothUUID Service::uuid() { return internal_->uuid(); }

ByteArray Service::data() { return internal_->data(); }

std::vector<Characteristic> Service::characteristics() { return internal_->characteristics(); }
